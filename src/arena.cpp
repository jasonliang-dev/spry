#include "arena.h"

struct ArenaNode {
  ArenaNode *next;
  u64 capacity;
  u64 allocd;
  u64 prev;
  u8 buf[1];
};

static u64 align_forward(u64 p, u32 align) {
  if ((p & (align - 1)) != 0) {
    p += align - (p & (align - 1));
  }
  return p;
}

static ArenaNode *arena_block_make(u64 capacity) {
  u64 page = 4096 - offsetof(ArenaNode, buf);
  if (capacity < page) {
    capacity = page;
  }

  ArenaNode *a = (ArenaNode *)mem_alloc(offsetof(ArenaNode, buf[capacity]));
  a->next = nullptr;
  a->allocd = 0;
  a->capacity = capacity;
  return a;
}

void Arena::trash() {
  ArenaNode *a = head;
  while (a != nullptr) {
    ArenaNode *rm = a;
    a = a->next;
    mem_free(rm);
  }
}

void *Arena::bump(u64 size) {
  if (head == nullptr) {
    head = arena_block_make(size);
  }

  u64 next = 0;
  do {
    next = align_forward(head->allocd, 16);
    if (next + size <= head->capacity) {
      break;
    }

    ArenaNode *block = arena_block_make(size);
    block->next = head;

    head = block;
  } while (true);

  void *ptr = &head->buf[next];
  head->allocd = next + size;
  head->prev = next;
  return ptr;
}

void *Arena::rebump(void *ptr, u64 old, u64 size) {
  if (head == nullptr || ptr == nullptr || old == 0) {
    return bump(size);
  }

  if (&head->buf[head->prev] == ptr) {
    u64 resize = head->prev + size;
    if (resize <= head->capacity) {
      head->allocd = resize;
      return ptr;
    }
  }

  void *new_ptr = bump(size);

  u64 copy = old < size ? old : size;
  memmove(new_ptr, ptr, copy);

  return new_ptr;
}

String Arena::bump_string(String s) {
  if (s.len > 0) {
    char *cstr = (char *)bump(s.len + 1);
    memcpy(cstr, s.data, s.len);
    cstr[s.len] = '\0';
    return {cstr, s.len};
  } else {
    return {};
  }
}
