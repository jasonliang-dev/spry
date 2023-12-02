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

  void trash() { mem_free(data); }

  void reserve(u64 cap) {
    if (cap > capacity) {
      T *buf = (T *)mem_alloc(sizeof(T) * cap);
      memcpy(buf, data, sizeof(T) * len);
      mem_free(data);
      data = buf;
      capacity = cap;
    }
  }

  void resize(u64 n) {
    reserve(n);
    len = n;
  }

  void push(T item) {
    if (len == capacity) {
      reserve(len > 0 ? len * 2 : 8);
    }
    data[len] = item;
    len++;
  }

  T *begin() { return data; }
  T *end() { return &data[len]; }
};
