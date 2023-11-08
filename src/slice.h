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
};

template <typename T>
void slice_from_arena(Slice<T> *s, Arena *arena, u64 len) {
  T *buf = (T *)arena_bump(arena, sizeof(T) * len);

  s->data = buf;
  s->len = len;
}

template <typename T> T *begin(const Slice<T> &s) { return s.data; }
template <typename T> T *end(const Slice<T> &s) { return &s.data[s.len]; }
