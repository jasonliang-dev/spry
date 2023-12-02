#pragma once

#include "arena.h"
#include "array.h"

template <typename T> struct Slice {
  T *data = nullptr;
  u64 len = 0;

  Slice() = default;
  Slice(Array<T> arr) : data(arr.data), len(arr.len) {}

  T &operator[](size_t i) {
    assert(i >= 0 && i < len);
    return data[i];
  }

  const T &operator[](size_t i) const {
    assert(i >= 0 && i < len);
    return data[i];
  }

  void resize(u64 n) {
    T *buf = (T *)mem_alloc(sizeof(T) * n);
    memcpy(buf, data, sizeof(T) * len);
    mem_free(data);
    data = buf;
    len = n;
  }

  void resize(Arena *arena, u64 n) {
    T *buf = (T *)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
    data = buf;
    len = n;
  }

  T *begin() { return data; }
  T *end() { return &data[len]; }
  const T *begin() const { return data; }
  const T *end() const { return &data[len]; }
};
