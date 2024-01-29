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

  {
    PROFILE_BLOCK("open libs");
    luaL_openlibs(L);
  }

  {
    PROFILE_BLOCK("open api");
    open_spry_api(L);
  }

  {
    PROFILE_BLOCK("open luasocket");
    open_luasocket(L);
  }

  {
    PROFILE_BLOCK("run bootstrap");
    luax_run_bootstrap(L);
  }

  String contents = lt->contents;

  {
    PROFILE_BLOCK("load chunk");
    if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) !=
        LUA_OK) {
      String err = luax_check_string(L, -1);
      fprintf(stderr, "%s\n", err.data);

      mem_free(contents.data);
      mem_free(lt->name.data);
      return;
    }
  }

  mem_free(contents.data);
  mem_free(lt->name.data);

  {
    PROFILE_BLOCK("run chunk");
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
      String err = luax_check_string(L, -1);
      fprintf(stderr, "%s\n", err.data);
    }
  }
}

void LuaThread::make(String code, String thread_name) {
  mtx.make();
  contents = to_cstr(code);
  name = to_cstr(thread_name);

  LockGuard lock{&mtx};
  thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
  if (LockGuard lock{&mtx}) {
    thread.join();
  }

  mtx.trash();
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
  case LUA_TUSERDATA: {
    i32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
    defer(lua_pop(L, 1));
    if (kind != LUA_TSTRING) {
      return;
    }

    kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
    defer(lua_pop(L, 1));
    if (kind != LUA_TNUMBER) {
      return;
    }

    String tname = luax_check_string(L, -2);
    u64 size = luaL_checkinteger(L, -1);

    if (size != sizeof(void *)) {
      return;
    }

    udata.ptr = *(void **)lua_touserdata(L, arg);
    udata.tname = to_cstr(tname);

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
  case LUA_TUSERDATA: {
    mem_free(udata.tname.data);
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
  case LUA_TUSERDATA: {
    luax_ptr_userdata(L, udata.ptr, udata.tname.data);
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
  mtx.make();
  sent.make();
  received.make();
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
  mtx.trash();
  sent.trash();
  received.trash();
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
  new (&chan->name) std::atomic<char *>();
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
  mtx.make();
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

void lua_channels_setup() {
  g_channels.select.make();
  g_channels.mtx.make();
}

void lua_channels_shutdown() {
  for (auto [k, v] : g_channels.by_name) {
    LuaChannel *chan = *v;
    chan->trash();
    mem_free(chan);
  }
  g_channels.by_name.trash();
  g_channels.select.trash();
  g_channels.mtx.trash();
}
