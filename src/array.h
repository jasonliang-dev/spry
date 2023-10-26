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

template <typename T> void drop(Array<T> *arr) { mem_free(arr->data); }

template <typename T> void reserve(Array<T> *arr, u64 capacity) {
  if (capacity > arr->capacity) {
    size_t bytes = sizeof(T) * capacity;
    T *buf = (T *)mem_alloc(bytes);
    memcpy(buf, arr->data, sizeof(T) * arr->len);
    mem_free(arr->data);
    arr->data = buf;
    arr->capacity = capacity;
  }
}

template <typename T> void resize(Array<T> *arr, u64 len) {
  reserve(arr, len);
  arr->len = len;
}

template <typename T> void push(Array<T> *arr, T item) {
  if (arr->len == arr->capacity) {
    reserve(arr, arr->capacity * 2 + 8);
  }
  arr->data[arr->len++] = item;
}

template <typename T> void pop(Array<T> *arr) {
  assert(arr->len != 0);
  arr->len--;
}

template <typename T> T *begin(Array<T> &arr) { return arr.data; }
template <typename T> T *end(Array<T> &arr) { return &arr.data[arr.len]; }
