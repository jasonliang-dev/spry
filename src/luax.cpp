#include "luax.h"
#include "app.h"
#include "profile.h"
#include "strings.h"
#include "vfs.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

i32 luax_require_script(lua_State *L, String filepath) {
  PROFILE_FUNC();

  if (g_app->error_mode) {
    return LUA_REFNIL;
  }

  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  String contents;
  bool ok = vfs_read_entire_file(&contents, filepath);
  if (!ok) {
    BUILD_STRING(sb);
    fatal_error(sb << "failed to read file: " << filepath);
    return LUA_REFNIL;
  }
  defer(mem_free(contents.data));

  lua_newtable(L);
  i32 module_table = lua_gettop(L);

  {
    PROFILE_BLOCK("load lua script");

    if (luaL_loadbuffer(L, contents.data, contents.len, path.data) != LUA_OK) {
      fatal_error(luax_check_string(L, -1));
      return LUA_REFNIL;
    }
  }

  // run script
  if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
    lua_pop(L, 2); // also pop module table
    return LUA_REFNIL;
  }

  // copy return results to module table
  i32 top = lua_gettop(L);
  for (i32 i = 1; i <= top - module_table; i++) {
    lua_seti(L, module_table, i);
  }

  return luaL_ref(L, LUA_REGISTRYINDEX);
}

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
  if (g_app->error_mode) {
    return 0;
  }

  String err = luax_check_string(L, -1);
  g_app->fatal_error = to_cstr(err);

  // traceback = debug.traceback(nil, 2)
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
  lua_pop(L, 2); // traceback and error
  return 0;
}

lua_Integer luax_len(lua_State *L, i32 arg) {
  lua_len(L, arg);
  lua_Integer len = luaL_checkinteger(L, -1);
  lua_pop(L, 1);
  return len;
}

void luax_geti(lua_State *L, i32 arg, lua_Integer n) {
  lua_pushinteger(L, n);
  lua_gettable(L, arg);
}

void luax_set_number_field(lua_State *L, const char *key, lua_Number n) {
  lua_pushnumber(L, n);
  lua_setfield(L, -2, key);
}

void luax_set_int_field(lua_State *L, const char *key, lua_Integer n) {
  lua_pushinteger(L, n);
  lua_setfield(L, -2, key);
}

void luax_set_string_field(lua_State *L, const char *key, const char *str) {
  lua_pushstring(L, str);
  lua_setfield(L, -2, key);
}

lua_Number luax_number_field(lua_State *L, i32 arg, const char *key) {
  lua_getfield(L, arg, key);
  lua_Number num = luaL_checknumber(L, -1);
  lua_pop(L, 1);
  return num;
}

lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key,
                                 lua_Number fallback) {
  i32 type = lua_getfield(L, arg, key);

  lua_Number num = fallback;
  if (type != LUA_TNIL) {
    num = luaL_optnumber(L, -1, fallback);
  }

  lua_pop(L, 1);
  return num;
}

lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key) {
  lua_getfield(L, arg, key);
  lua_Integer num = luaL_checkinteger(L, -1);
  lua_pop(L, 1);
  return num;
}

lua_Number luax_opt_int_field(lua_State *L, i32 arg, const char *key,
                              lua_Number fallback) {
  i32 type = lua_getfield(L, arg, key);

  lua_Number num = fallback;
  if (type != LUA_TNIL) {
    num = luaL_optinteger(L, -1, fallback);
  }

  lua_pop(L, 1);
  return num;
}

String luax_string_field(lua_State *L, i32 arg, const char *key) {
  lua_getfield(L, arg, key);
  size_t len = 0;
  char *str = (char *)luaL_checklstring(L, -1, &len);
  lua_pop(L, 1);
  return {str, len};
}

String luax_opt_string_field(lua_State *L, i32 arg, const char *key,
                             const char *fallback) {
  lua_getfield(L, arg, key);
  size_t len = 0;
  char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
  lua_pop(L, 1);
  return {str, len};
}

bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback) {
  i32 type = lua_getfield(L, arg, key);

  bool b = fallback;
  if (type != LUA_TNIL) {
    b = lua_toboolean(L, -1);
  }

  lua_pop(L, 1);
  return b;
}

String luax_check_string(lua_State *L, i32 arg) {
  size_t len = 0;
  char *str = (char *)luaL_checklstring(L, arg, &len);
  return {str, len};
}

String luax_opt_string(lua_State *L, i32 arg, String def) {
  return lua_isstring(L, arg) ? luax_check_string(L, arg) : def;
}

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack,
                      String needle) {
  BUILD_STRING(sb);

  sb << "expected one of: {";
  for (String s : haystack) {
    sb << "\"" << s << "\", ";
  }
  if (haystack.size() != 0) {
    sb.len -= 2;
  }
  sb << "} got: \"" << needle << "\".";

  return luaL_error(L, "%s", sb.data);
}

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
  luaL_newmetatable(L, mt_name);
  luaL_setfuncs(L, l, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}
