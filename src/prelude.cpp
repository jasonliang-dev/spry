#include "prelude.h"

void *DebugAllocator::alloc(size_t bytes, const char *file, i32 line) {
  LockGuard lock{&mtx};

  DebugAllocInfo *info =
      (DebugAllocInfo *)malloc(offsetof(DebugAllocInfo, buf[bytes]));
  info->file = file;
  info->line = line;
  info->size = bytes;
  info->prev = nullptr;
  info->next = head;
  if (head != nullptr) {
    head->prev = info;
  }
  head = info;
  return info->buf;
}

void DebugAllocator::free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }

  LockGuard lock{&mtx};

  DebugAllocInfo *info =
      (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

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

void DebugAllocator::dump_allocs() {
  i32 allocs = 0;
  for (DebugAllocInfo *info = head; info != nullptr; info = info->next) {
    printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size,
           info->file, info->line);
    allocs++;
  }
  printf("  --- %d allocation(s) ---\n", allocs);
}

bool String::is_cstr() { return data[len] == '\0'; }

String String::substr(u64 i, u64 j) {
  assert(i <= j);
  assert(j <= (i64)len);
  return {&data[i], j - i};
}

bool String::starts_with(String match) {
  if (len < match.len) {
    return false;
  }
  return substr(0, match.len) == match;
}

bool String::ends_with(String match) {
  if (len < match.len) {
    return false;
  }
  return substr(len - match.len, len) == match;
}

u64 String::first_of(char c) {
  for (u64 i = 0; i < len; i++) {
    if (data[i] == c) {
      return i;
    }
  }

  return (u64)-1;
}

u64 String::last_of(char c) {
  for (u64 i = len; i > 0; i--) {
    if (data[i - 1] == c) {
      return i;
    }
  }

  return (u64)-1;
}
