#pragma once

#include "array.h"
#include "hash_map.h"
#include "strings.h"

typedef enum {
  JsonKind_Null,
  JsonKind_Object,
  JsonKind_Array,
  JsonKind_String,
  JsonKind_Number,
  JsonKind_Boolean,
} JSONKind;

struct JSON {
  union {
    HashMap<JSON> object;
    Array<JSON> array;
    String string;
    double number;
    bool boolean;
  };
  JSONKind kind;
};

String json_parse(JSON *out, String contents);
void json_trash(JSON *json);
JSON json_lookup(JSON obj, String key);
JSON json_index(JSON arr, i32 index);
HashMap<JSON> json_object(JSON json);
Array<JSON> json_array(JSON json);
String json_string(JSON json);
double json_number(JSON json);

void json_write_string(StringBuilder *sb, JSON json);
void json_print(JSON json);