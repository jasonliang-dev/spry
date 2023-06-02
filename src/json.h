#pragma once

#include "deps/json.h"
#include "prelude.h"
#include "scanner.h"

using ObjectEl = json_object_element_t;
using ArrayEl = json_array_element_t;

struct JsonArrayIterator {
  ArrayEl *el = nullptr;

  ArrayEl *operator*() const { return el; }

  JsonArrayIterator &operator++() {
    el = el->next;
    return *this;
  }
};

inline bool operator!=(JsonArrayIterator lhs, JsonArrayIterator rhs) {
  return lhs.el != rhs.el;
}

struct JsonObjectIterator {
  ObjectEl *el = nullptr;

  ObjectEl *operator*() const { return el; }

  JsonObjectIterator &operator++() {
    el = el->next;
    return *this;
  }
};

inline bool operator!=(JsonObjectIterator lhs, JsonObjectIterator rhs) {
  return lhs.el != rhs.el;
}

inline JsonArrayIterator begin(json_array_t *arr) { return {arr->start}; }
inline JsonArrayIterator end(json_array_t *) { return {nullptr}; }

inline JsonObjectIterator begin(json_object_t *obj) { return {obj->start}; }
inline JsonObjectIterator end(json_object_t *) { return {nullptr}; }

inline json_object_t *as_object(json_value_t *value) {
  return json_value_as_object(value);
}

inline json_array_t *as_array(json_value_t *value) {
  return json_value_as_array(value);
}

inline String as_string(json_value_t *value) {
  if (value->type != json_type_string) {
    return ""_str;
  }

  json_string_t *str = (json_string_t *)value->payload;
  return {(char *)str->string, str->string_size};
}

inline String as_string(json_string_t *str) {
  return {(char *)str->string, str->string_size};
}

inline i32 as_int(json_value_s *value) {
  if (value->type != json_type_number) {
    return 0;
  }

  json_number_t *num = (json_number_t *)value->payload;
  String str = {(char *)num->number, num->number_size};

  Scanner scan = make_scanner(str);
  return next_int(&scan);
}

inline u64 hash(json_string_t *str) {
  return fnv1a(str->string, str->string_size);
}
