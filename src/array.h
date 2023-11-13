#pragma once

#include "prelude.h"

template <typename T> struct Array {
  T *data = nullptr;
  u64 len = 0;
  u64 capacity = 0;

  T &operator[](size_t i) {
    assert(i >= 0 && i < len);
    return data[i];
  }
};

template <typename T> void array_trash(Array<T> *arr) { mem_free(arr->data); }

template <typename T> void array_reserve(Array<T> *arr, u64 capacity) {
  if (capacity > arr->capacity) {
    T *buf = (T *)mem_alloc(sizeof(T) * capacity);
    memcpy(buf, arr->data, sizeof(T) * arr->len);
    mem_free(arr->data);
    arr->data = buf;
    arr->capacity = capacity;
  }
}

template <typename T> void array_resize(Array<T> *arr, u64 len) {
  array_reserve(arr, len);
  arr->len = len;
}

template <typename T> void array_push(Array<T> *arr, T item) {
  i32 len = arr->len;
  if (len == arr->capacity) {
    i32 grow = len > 0 ? len * 2 : 8;
    array_reserve(arr, grow);
  }
  arr->data[len] = item;
  arr->len++;
}

template <typename T> T *begin(Array<T> &arr) { return arr.data; }
template <typename T> T *end(Array<T> &arr) { return &arr.data[arr.len]; }
