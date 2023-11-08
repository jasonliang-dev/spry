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
  JSON *parent;
  JSONKind kind;
  bool had_error;
};

struct JSONObject {
  JSON value;
  JSONObject *next;
  u64 key;
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
};

void json_parse(JSONDocument *out, String contents);
void json_trash(JSONDocument *doc);
JSON *json_lookup(JSON *obj, String key);
JSON *json_index(JSON *arr, i32 index);
JSONObject *json_object(JSON *json);
JSONArray *json_array(JSON *json);
String json_string(JSON *json);
double json_number(JSON *json);

inline String json_lookup_string(JSON *json, String key) {
  return json_string(json_lookup(json, key));
}

inline double json_lookup_number(JSON *json, String key) {
  return json_number(json_lookup(json, key));
}

struct StringBuilder;
void json_write_string(StringBuilder *sb, JSON *json);
void json_print(JSON *json);