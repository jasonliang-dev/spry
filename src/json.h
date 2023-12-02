#pragma once

#include "arena.h"

enum JSONKind : i32 {
  JSONKind_Null,
  JSONKind_Object,
  JSONKind_Array,
  JSONKind_String,
  JSONKind_Number,
  JSONKind_Boolean,
};

struct JSONObject;
struct JSONArray;
struct JSON {
  union {
    JSONObject *object;
    JSONArray *array;
    String string;
    double number;
    bool boolean;
  };
  JSONKind kind;

  JSON lookup(String key, bool *ok);
  JSON index(i32 i, bool *ok);

  JSONObject *as_object(bool *ok);
  JSONArray *as_array(bool *ok);
  String as_string(bool *ok);
  double as_number(bool *ok);

  JSONObject *lookup_object(String key, bool *ok);
  JSONArray *lookup_array(String key, bool *ok);
  String lookup_string(String key, bool *ok);
  double lookup_number(String key, bool *ok);

  double index_number(i32 i, bool *ok);
};

struct JSONObject {
  JSON value;
  String key;
  JSONObject *next;
  u64 hash;
};

struct JSONArray {
  JSON value;
  JSONArray *next;
  u64 index;
};

struct JSONDocument {
  JSON root;
  String error;
  Arena arena;

  void parse(String contents);
  void trash();
};

struct StringBuilder;
void json_write_string(StringBuilder *sb, JSON *json);
void json_print(JSON *json);

struct lua_State;
void json_to_lua(lua_State *L, JSON *json);
void lua_to_json_string(lua_State *L, i32 arg, String *contents, String *err);
