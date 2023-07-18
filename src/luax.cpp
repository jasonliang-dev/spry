#include "luax.h"
#include "app.h"
#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"

void luax_stack_dump(lua_State *L) {
  i32 top = lua_gettop(L);
  printf("  --- lua stack (%d) ---\n", top);
  for (i32 i = 1; i <= top; i++) {
    printf("  [%d] (%s): ", i, luaL_typename(L, i));

    switch (lua_type(L, i)) {
    case LUA_TNUMBER: printf("%f\n", lua_tonumber(L, i)); break;
    case LUA_TSTRING: printf("%s\n", lua_tostring(L, i)); break;
    case LUA_TBOOLEAN: printf("%d\n", lua_toboolean(L, i)); break;
    case LUA_TNIL: printf("nil\n"); break;
    default: printf("%p\n", lua_topointer(L, i)); break;
    }
  }
}

int luax_msgh(lua_State *L) {
  String err = luax_check_string(L, -1);
  g_app->fatal_error = to_cstr(err);

  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_remove(L, -2);
  lua_pushnil(L);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  String traceback = luax_check_string(L, -1);
  g_app->traceback = to_cstr(traceback);

  for (u64 i = 0; i < g_app->traceback.len; i++) {
    if (g_app->traceback.data[i] == '\t') {
      g_app->traceback.data[i] = ' ';
    }
  }

  fprintf(stderr, "%s\n%s\n", g_app->fatal_error.data, g_app->traceback.data);

  g_app->error_mode = true;
  lua_pop(L, 2);
  return 0;
}

void luax_set_field(lua_State *L, const char *key, lua_Number n) {
  lua_pushnumber(L, n);
  lua_setfield(L, -2, key);
}

void luax_set_field(lua_State *L, const char *key, const char *str) {
  lua_pushstring(L, str);
  lua_setfield(L, -2, key);
}

lua_Number luax_number_field(lua_State *L, const char *key) {
  lua_getfield(L, -1, key);
  lua_Number num = luaL_checknumber(L, -1);
  lua_pop(L, 1);
  return num;
}

lua_Number luax_number_field(lua_State *L, const char *key,
                             lua_Number fallback) {
  i32 type = lua_getfield(L, -1, key);

  lua_Number num = fallback;
  if (type != LUA_TNIL) {
    num = luaL_optnumber(L, -1, fallback);
  }

  lua_pop(L, 1);
  return num;
}

String luax_string_field(lua_State *L, const char *key) {
  lua_getfield(L, -1, key);
  usize len;
  char *str = (char *)luaL_checklstring(L, -1, &len);
  lua_pop(L, 1);
  return {str, len};
}

String luax_string_field(lua_State *L, const char *key, const char *fallback) {
  lua_getfield(L, -1, key);
  usize len;
  char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
  lua_pop(L, 1);
  return {str, len};
}

bool luax_boolean_field(lua_State *L, const char *key, bool fallback) {
  i32 type = lua_getfield(L, -1, key);

  bool b = fallback;
  if (type != LUA_TNIL) {
    b = lua_toboolean(L, -1);
  }

  lua_pop(L, 1);
  return b;
}

String luax_check_string(lua_State *L, i32 arg) {
  usize len;
  char *str = (char *)luaL_checklstring(L, arg, &len);
  return {str, len};
}

String luax_opt_string(lua_State *L, i32 arg, String def) {
  return lua_isstring(L, arg) ? luax_check_string(L, arg) : def;
}

DrawDescription luax_draw_description(lua_State *L, i32 arg_start) {
  DrawDescription dd;

  dd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
  dd.y = (float)luaL_optnumber(L, arg_start + 1, 0);

  dd.rotation = (float)luaL_optnumber(L, arg_start + 2, 0);

  dd.sx = (float)luaL_optnumber(L, arg_start + 3, 1);
  dd.sy = (float)luaL_optnumber(L, arg_start + 4, 1);

  dd.ox = (float)luaL_optnumber(L, arg_start + 5, 0);
  dd.oy = (float)luaL_optnumber(L, arg_start + 6, 0);

  dd.u0 = (float)luaL_optnumber(L, arg_start + 7, 0);
  dd.v0 = (float)luaL_optnumber(L, arg_start + 8, 0);
  dd.u1 = (float)luaL_optnumber(L, arg_start + 9, 1);
  dd.v1 = (float)luaL_optnumber(L, arg_start + 10, 1);

  return dd;
}

RectDescription luax_rect_description(lua_State *L, i32 arg_start) {
  RectDescription rd;

  rd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
  rd.y = (float)luaL_optnumber(L, arg_start + 1, 0);
  rd.w = (float)luaL_optnumber(L, arg_start + 2, 0);
  rd.h = (float)luaL_optnumber(L, arg_start + 3, 0);

  rd.rotation = (float)luaL_optnumber(L, arg_start + 4, 0);

  rd.sx = (float)luaL_optnumber(L, arg_start + 5, 1);
  rd.sy = (float)luaL_optnumber(L, arg_start + 6, 1);

  rd.ox = (float)luaL_optnumber(L, arg_start + 7, 0);
  rd.oy = (float)luaL_optnumber(L, arg_start + 8, 0);

  return rd;
}

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
  luaL_newmetatable(L, mt_name);
  luaL_setfuncs(L, l, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
}
