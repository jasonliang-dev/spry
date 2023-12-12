#include "concurrency.h"
#include "api.h"
#include "deps/luaalloc.h"
#include "hash_map.h"
#include "luax.h"
#include "prelude.h"
#include "profile.h"
#include "sync.h"
#include <new>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

static void lua_thread_proc(void *udata) {
  PROFILE_FUNC();

  LuaThread *lt = (LuaThread *)udata;

  LuaAlloc *LA = luaalloc_create(nullptr, nullptr);
  defer(luaalloc_delete(LA));

  lua_State *L = lua_newstate(luaalloc, LA);
  defer(lua_close(L));

  luaL_openlibs(L);
  open_spry_api(L);
  open_luasocket(L);
  luax_run_bootstrap(L);

  String contents = lt->contents;

  if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) !=
      LUA_OK) {
    String err = luax_check_string(L, -1);
    fprintf(stderr, "%s\n", err.data);

    mem_free(contents.data);
    mem_free(lt->name.data);
    return;
  }

  mem_free(contents.data);
  mem_free(lt->name.data);

  if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
    String err = luax_check_string(L, -1);
    fprintf(stderr, "%s\n", err.data);
  }
}

void LuaThread::make(String code, String thread_name) {
  contents = to_cstr(code);
  name = to_cstr(thread_name);

  Thread t = {};
  t.make(lua_thread_proc, this);
  thread.store(t);
}

void LuaThread::join() {
  Thread t = thread.exchange({});
  t.join();
}

//

void LuaVariant::make(lua_State *L, i32 arg) {
  type = lua_type(L, arg);

  switch (type) {
  case LUA_TBOOLEAN: boolean = lua_toboolean(L, arg); break;
  case LUA_TNUMBER: number = luaL_checknumber(L, arg); break;
  case LUA_TSTRING: {
    String s = luax_check_string(L, arg);
    string = to_cstr(s);
    break;
  }
  case LUA_TTABLE: {
    Array<LuaTableEntry> entries = {};
    entries.resize(luax_len(L, arg));

    lua_pushvalue(L, arg);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      LuaVariant key = {};
      key.make(L, -2);

      LuaVariant value = {};
      value.make(L, -1);

      entries.push({key, value});
    }
    lua_pop(L, 1);

    table = Slice(entries);
    break;
  }
  default: break;
  }
}

void LuaVariant::trash() {
  switch (type) {
  case LUA_TSTRING: {
    mem_free(string.data);
    break;
  }
  case LUA_TTABLE: {
    for (LuaTableEntry e : table) {
      e.key.trash();
      e.value.trash();
    }
    mem_free(table.data);
  }
  default: break;
  }
}

void LuaVariant::push(lua_State *L) {
  switch (type) {
  case LUA_TBOOLEAN: lua_pushboolean(L, boolean); break;
  case LUA_TNUMBER: lua_pushnumber(L, number); break;
  case LUA_TSTRING: lua_pushlstring(L, string.data, string.len); break;
  case LUA_TTABLE: {
    lua_newtable(L);
    for (LuaTableEntry e : table) {
      e.key.push(L);
      e.value.push(L);
      lua_rawset(L, -3);
    }
    break;
  }
  default: break;
  }
}

//

struct LuaChannels {
  Mutex mtx;
  Cond select;
  HashMap<LuaChannel *> by_name;
};

static LuaChannels g_channels = {};

void LuaChannel::make(String n, u64 buf) {
  new (this) LuaChannel();
  items.data = (LuaVariant *)mem_alloc(sizeof(LuaVariant) * (buf + 1));
  items.len = (buf + 1);
  front = 0;
  back = 0;
  len = 0;

  name.store(to_cstr(n).data);
}

void LuaChannel::trash() {
  for (i32 i = 0; i < len; i++) {
    items[front].trash();
    front = (front + 1) % items.len;
  }

  mem_free(items.data);
  mem_free(name.exchange(nullptr));
  this->~LuaChannel();
  mem_free(this);
}

void LuaChannel::send(LuaVariant item) {
  LockGuard lock{&mtx};

  while (len == items.len) {
    received.wait(&mtx);
  }

  items[back] = item;
  back = (back + 1) % items.len;
  len++;

  g_channels.select.broadcast();
  sent.signal();
  sent_total++;

  while (sent_total >= received_total + items.len) {
    received.wait(&mtx);
  }
}

static LuaVariant lua_channel_dequeue(LuaChannel *ch) {
  LuaVariant item = ch->items[ch->front];
  ch->front = (ch->front + 1) % ch->items.len;
  ch->len--;

  ch->received.broadcast();
  ch->received_total++;

  return item;
}

LuaVariant LuaChannel::recv() {
  LockGuard lock{&mtx};

  while (len == 0) {
    sent.wait(&mtx);
  }

  return lua_channel_dequeue(this);
}

bool LuaChannel::try_recv(LuaVariant *v) {
  LockGuard lock{&mtx};

  if (len == 0) {
    return false;
  }

  *v = lua_channel_dequeue(this);
  return true;
}

LuaChannel *lua_channel_make(String name, u64 buf) {
  LuaChannel *chan = (LuaChannel *)mem_alloc(sizeof(LuaChannel));
  chan->make(name, buf);

  LockGuard lock{&g_channels.mtx};
  g_channels.by_name[fnv1a(name)] = chan;

  return chan;
}

LuaChannel *lua_channel_get(String name) {
  LockGuard lock{&g_channels.mtx};

  LuaChannel **chan = g_channels.by_name.get(fnv1a(name));
  if (chan == nullptr) {
    return nullptr;
  }

  return *chan;
}

LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v) {
  i32 len = lua_gettop(L);
  if (len == 0) {
    return nullptr;
  }

  LuaChannel *buf[16] = {};
  for (i32 i = 0; i < len; i++) {
    buf[i] = *(LuaChannel **)luaL_checkudata(L, i + 1, "mt_channel");
  }

  Mutex mtx = {};
  LockGuard lock{&mtx};

  while (true) {
    for (i32 i = 0; i < len; i++) {
      LockGuard lock{&buf[i]->mtx};
      if (buf[i]->len > 0) {
        *v = lua_channel_dequeue(buf[i]);
        return buf[i];
      }
    }

    g_channels.select.wait(&mtx);
  }
}

void lua_channels_shutdown() {
  for (auto [k, v] : g_channels.by_name) {
    LuaChannel *chan = *v;
    chan->trash();
  }
  g_channels.by_name.trash();
}
