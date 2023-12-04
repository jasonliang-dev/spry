#include "concurrency.h"
#include "api.h"
#include "app.h"
#include "deps/luaalloc.h"
#include "hash_map.h"
#include "luax.h"
#include "prelude.h"
#include "profile.h"
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

  String contents = lt->contents;
  defer(mem_free(contents.data));

  if (luaL_loadbuffer(L, contents.data, contents.len, nullptr) != LUA_OK) {
    lt->error = to_cstr(luax_check_string(L, -1));
    lua_pop(L, 1);
    return;
  }

  if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
    lt->error = to_cstr(luax_check_string(L, -1));
    lua_pop(L, 1);
  }
}

LuaThread *lua_thread_make(String contents) {
  LuaThread *lt = (LuaThread *)mem_alloc(sizeof(LuaThread));
  *lt = {};
  lt->contents = to_cstr(contents);

  Thread t = {};
  t.make(lua_thread_proc, lt);
  lt->thread = t.ptr;

  return lt;
}

void LuaThread::join() {
  Thread system_thread = {thread};
  system_thread.join();
}

//

LuaThreadValue lua_thread_value(lua_State *L, i32 arg) {
  LuaThreadValue v = {};
  v.type = lua_type(L, arg);

  switch (v.type) {
  case LUA_TNUMBER: v.number = luaL_checknumber(L, arg); break;
  case LUA_TSTRING: {
    String s = luax_check_string(L, arg);
    v.string = to_cstr(s);
    break;
  }
  default: v = {}; break;
  }

  return v;
}

//

LuaChannel *lua_channel_make(u64 cap) {
  LuaChannel *chan = (LuaChannel *)mem_alloc(sizeof(LuaChannel));
  new (chan) LuaChannel();

  chan->items.data =
      (LuaThreadValue *)mem_alloc(sizeof(LuaThreadValue) * cap);
  chan->items.len = cap;
  chan->front = 0;
  chan->back = 0;
  chan->len = 0;

  return chan;
}

void LuaChannel::trash() {
  for (i32 i = 0; i < len; i++) {
    LuaThreadValue v = items[front];
    if (v.type == LUA_TSTRING) {
      mem_free(v.string.data);
    }

    front = (front + 1) % items.len;
  }

  mem_free(items.data);
}

void LuaChannel::send(LuaThreadValue item) {
  LockGuard lock{&mtx};

  while (len == items.len) {
    received.wait(&mtx);
  }

  items[back] = item;
  back = (back + 1) % items.len;
  len++;

  sent.signal();
}

LuaThreadValue LuaChannel::recv() {
  LockGuard lock{&mtx};

  while (len == 0) {
    sent.wait(&mtx);
  }

  LuaThreadValue item = items[front];
  front = (front + 1) % items.len;
  len--;

  received.signal();
  return item;
}

LuaChannel *lua_channel_get(String name, u64 cap) {
  LockGuard lock{&g_app->channels.mtx};

  LuaChannel **chan = nullptr;
  bool exist = g_app->channels.by_name.find_or_insert(fnv1a(name), &chan);
  if (!exist) {
    *chan = lua_channel_make(cap);
  }

  return *chan;
}
