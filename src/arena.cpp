#include "arena.h"

struct ArenaNode {
  ArenaNode *next;
  u64 capacity;
  u64 allocd;
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

void arena_trash(Arena *arena) {
  ArenaNode *a = arena->head;
  while (a != nullptr) {
    ArenaNode *rm = a;
    a = a->next;
    mem_free(rm);
  }
}

void *arena_bump(Arena *arena, u64 size) {
  if (arena->head == nullptr) {
    arena->head = arena_block_make(size);
  }

  u64 next = 0;
  do {
    next = align_forward(arena->head->allocd, 16);
    if (next + size <= arena->head->capacity) {
      break;
    }

    ArenaNode *block = arena_block_make(size);
    block->next = arena->head;

    arena->head = block;
  } while (true);

  void *ptr = &arena->head->buf[next];
  arena->head->allocd = next + size;
  return ptr;
}

String arena_bump_string(Arena *arena, String s) {
  if (s.len > 0) {
    char *cstr = (char *)arena_bump(arena, s.len + 1);
    memcpy(cstr, s.data, s.len + 1);
    cstr[s.len] = '\0';
    return {cstr, s.len};
  } else {
    return {};
  }
}
