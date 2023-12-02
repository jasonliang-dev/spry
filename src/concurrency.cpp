#include "concurrency.h"
#include "api.h"
#include "deps/luaalloc.h"
#include "luax.h"
#include "profile.h"

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
  open_luasocket(L);

  String contents = lt->contents;
  if (luaL_loadbuffer(L, contents.data, contents.len, nullptr) != LUA_OK) {
    lt->error = to_cstr(luax_check_string(L, -1));
    lua_pop(L, 1);
  }
  mem_free(contents.data);

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
  mem_free(this);
}
