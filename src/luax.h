#pragma once

#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"
#include "prelude.h"
#include <initializer_list>

i32 luax_require_script(lua_State *L, String filepath);

void luax_stack_dump(lua_State *L);

// message handler. prints error and traceback
int luax_msgh(lua_State *L);

// set table value at top of stack
void luax_set_field(lua_State *L, const char *key, lua_Number n);
void luax_set_field(lua_State *L, const char *key, const char *str);

// get value from table at top of stack
lua_Number luax_number_field(lua_State *L, const char *key);
lua_Number luax_number_field(lua_State *L, const char *key,
                             lua_Number fallback);
String luax_string_field(lua_State *L, const char *key);
String luax_string_field(lua_State *L, const char *key, const char *fallback);
bool luax_boolean_field(lua_State *L, const char *key, bool fallback = false);

String luax_check_string(lua_State *L, i32 arg);
String luax_opt_string(lua_State *L, i32 arg, String def);

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack,
                      String needle);
void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l);

#define luax_new_userdata(L, data, tname)                                      \
  do {                                                                         \
    void *udata = lua_newuserdatauv(L, sizeof(data), 0);                       \
    memcpy(udata, &data, sizeof(data));                                        \
    luaL_setmetatable(L, tname);                                               \
  } while (0)

#define luax_ptr_userdata(L, data, tname)                                      \
  do {                                                                         \
    void *udata = lua_newuserdatauv(L, sizeof(void *), 0);                     \
    memcpy(udata, &data, sizeof(void *));                                      \
    luaL_setmetatable(L, tname);                                               \
  } while (0)
