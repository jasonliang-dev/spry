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
};

template <typename T> void slice_from_len(Slice<T> *s, u64 len) {
  T *buf = (T *)mem_alloc(sizeof(T) * len);

  s->data = buf;
  s->len = len;
}

template <typename T>
void slice_from_arena(Slice<T> *s, Arena *arena, u64 len) {
  T *buf = (T *)arena_bump(arena, sizeof(T) * len);

  s->data = buf;
  s->len = len;
}

template <typename T> u64 slice_resize(Slice<T> *s, Arena *arena, u64 len) {
  T *buf =
      (T *)arena_rebump(arena, s->data, sizeof(T) * s->len, sizeof(T) * len);

  s->data = buf;
  s->len = len;

  return len;
}

template <typename T> T *begin(Slice<T> &s) { return s.data; }
template <typename T> T *end(Slice<T> &s) { return &s.data[s.len]; }
template <typename T> const T *begin(const Slice<T> &s) { return s.data; }
template <typename T> const T *end(const Slice<T> &s) { return &s.data[s.len]; }
