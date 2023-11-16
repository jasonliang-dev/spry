#pragma once

#include "deps/cute_sync.h"
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

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

struct Allocator {
  virtual ~Allocator() = 0;
  virtual void make() = 0;
  virtual void trash() = 0;
  virtual void *alloc(size_t bytes, const char *file, i32 line) = 0;
  virtual void free(void *ptr) = 0;
};
inline Allocator::~Allocator() {}

struct HeapAllocator : Allocator {
  void make() {}
  void trash() {}
  void *alloc(size_t bytes, const char *, i32) { return malloc(bytes); }
  void free(void *ptr) { ::free(ptr); }
};

struct DebugAllocInfo {
  const char *file;
  i32 line;
  size_t size;
  DebugAllocInfo *prev;
  DebugAllocInfo *next;
};

struct DebugAllocator : Allocator {
  DebugAllocInfo *head = nullptr;
  cute_mutex_t mtx = {};

  void make() { mtx = cute_mutex_create(); }
  void trash() { cute_mutex_destroy(&mtx); }

  void *alloc(size_t bytes, const char *file, i32 line) {
    cute_lock(&mtx);
    defer(cute_unlock(&mtx));

    DebugAllocInfo *info =
        (DebugAllocInfo *)malloc(sizeof(DebugAllocInfo) + bytes);
    info->file = file;
    info->line = line;
    info->size = bytes;
    info->prev = nullptr;
    info->next = head;
    if (head != nullptr) {
      head->prev = info;
    }
    head = info;
    return info + 1;
  }

  void free(void *ptr) {
    if (ptr == nullptr) {
      return;
    }

    cute_lock(&mtx);
    defer(cute_unlock(&mtx));

    DebugAllocInfo *info = (DebugAllocInfo *)ptr - 1;

    if (info->prev == nullptr) {
      head = info->next;
    } else {
      info->prev->next = info->next;
    }

    if (info->next) {
      info->next->prev = info->prev;
    }

    ::free(info);
  }
};

extern Allocator *g_allocator;

#define mem_alloc(bytes) g_allocator->alloc(bytes, __FILE__, __LINE__)
#define mem_free(ptr) g_allocator->free(ptr)

inline bool is_whitespace(char c) {
  switch (c) {
  case '\n':
  case '\r':
  case '\t':
  case ' ': return true;
  }
  return false;
}

inline bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

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
