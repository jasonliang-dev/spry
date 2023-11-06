#pragma once

#include "array.h"
#include "hash_map.h"
#include "strings.h"

enum JSONKind : i32 {
  JSONKind_Null,
  JSONKind_Object,
  JSONKind_Array,
  JSONKind_String,
  JSONKind_Number,
  JSONKind_Boolean,
};

struct JSON {
  union {
    HashMap<JSON> object;
    Array<JSON> array;
    String string;
    double number;
    bool boolean;
  };
  JSON *parent;
  JSONKind kind;
  bool had_error;
};

String json_parse(JSON *out, String contents);
void json_trash(JSON *json);
JSON *json_lookup(JSON *obj, String key);
JSON *json_index(JSON *arr, i32 index);
HashMap<JSON> json_object(JSON *json);
Array<JSON> json_array(JSON *json);
String json_string(JSON *json);
double json_number(JSON *json);

inline String json_lookup_string(JSON *json, String key) {
  return json_string(json_lookup(json, key));
}

inline double json_lookup_number(JSON *json, String key) {
  return json_number(json_lookup(json, key));
}

void json_write_string(StringBuilder *sb, JSON *json);
void json_print(JSON *json);