#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_size(a) (sizeof(a) / sizeof(a[0]))
#define MATH_PI 3.1415926535897f

#ifdef __clang__
#define FORMAT_ARGS(n) __attribute__((format(printf, n, n + 1)))
#else
#define FORMAT_ARGS(n)
#endif

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using usize = size_t;
using isize = ptrdiff_t;

inline i32 min(i32 a, i32 b) { return a < b ? a : b; }
inline i32 max(i32 a, i32 b) { return a > b ? a : b; }

struct DebugAllocInfo {
  const char *file;
  i32 line;
  size_t size;
  DebugAllocInfo *prev;
  DebugAllocInfo *next;
};

struct Allocator {
  void *(*alloc)(Allocator *a, size_t bytes, const char *file, i32 line);
  void (*free)(Allocator *a, void *ptr);
  DebugAllocInfo *head;
};

inline void *heap_alloc(Allocator *, size_t bytes, const char *, i32) {
  return malloc(bytes);
}

inline void heap_free(Allocator *, void *ptr) { free(ptr); }

inline void *debug_alloc(Allocator *a, size_t bytes, const char *file,
                         i32 line) {
  DebugAllocInfo *info =
      (DebugAllocInfo *)malloc(sizeof(DebugAllocInfo) + bytes);
  info->file = file;
  info->line = line;
  info->size = bytes;
  info->prev = nullptr;
  info->next = a->head;
  if (a->head != nullptr) {
    a->head->prev = info;
  }
  a->head = info;
  return info + 1;
}

inline void debug_free_nothing(Allocator *, void *) {}

inline void debug_free(Allocator *a, void *ptr) {
  if (ptr == nullptr) {
    return;
  }

  DebugAllocInfo *info = (DebugAllocInfo *)ptr - 1;

  if (info->prev == nullptr) {
    a->head = info->next;
  } else {
    info->prev->next = info->next;
  }

  if (info->next) {
    info->next->prev = info->prev;
  }

  free(info);
}

inline Allocator heap_allocator() {
  Allocator a = {};
  a.alloc = heap_alloc;
  a.free = heap_free;
  return a;
}

inline Allocator debug_allocator() {
  Allocator a = {};
  a.alloc = debug_alloc;
  a.free = debug_free;
  return a;
}

inline Allocator debug_no_free_allocator() {
  Allocator a = {};
  a.alloc = debug_alloc;
  a.free = debug_free_nothing;
  return a;
}

extern Allocator g_allocator;

#define mem_alloc(bytes)                                                       \
  g_allocator.alloc(&g_allocator, bytes, __FILE__, __LINE__)
#define mem_free(ptr) g_allocator.free(&g_allocator, ptr)

struct String {
  char *data = nullptr;
  u64 len = 0;

  String() = default;
  String(const char *cstr) : data((char *)cstr), len(strlen(cstr)) {}
  String(const char *cstr, u64 n) : data((char *)cstr), len(n) {}
};

inline String to_cstr(String str) {
  char *buf = (char *)mem_alloc(str.len + 1);
  memcpy(buf, str.data, str.len);
  buf[str.len] = 0;
  return {buf, str.len};
}

inline char *begin(String str) { return str.data; }
inline char *end(String str) { return &str.data[str.len]; }

constexpr u64 fnv1a(const char *str, u64 len) {
  u64 hash = 14695981039346656037u;
  for (u64 i = 0; i < len; i++) {
    hash ^= (u8)str[i];
    hash *= 1099511628211;
  }
  return hash;
}

inline u64 fnv1a(String str) { return fnv1a(str.data, str.len); }

constexpr u64 operator"" _hash(const char *str, size_t len) {
  return fnv1a(str, len);
}

inline bool operator==(String lhs, String rhs) {
  if (lhs.len != rhs.len) {
    return false;
  }
  return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

inline bool operator!=(String lhs, String rhs) { return !(lhs == rhs); }

#define JOIN_1(x, y) x##y
#define JOIN_2(x, y) JOIN_1(x, y)

template <typename F> struct Defer {
  F f;
  Defer(F f) : f(f) {}
  ~Defer() { f(); }
};

template <typename F> Defer<F> defer_func(F f) { return Defer<F>(f); }

#define defer(code)                                                            \
  auto JOIN_2(_defer_, __COUNTER__) = defer_func([&]() { code; })