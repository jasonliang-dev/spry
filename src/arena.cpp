#include "arena.h"

struct ArenaBlock {
  ArenaBlock *next;
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

static ArenaBlock *arena_block_make(u64 capacity) {
  u64 page = 4096 - offsetof(ArenaBlock, buf);
  if (capacity < page) {
    capacity = page;
  }

  ArenaBlock *a = (ArenaBlock *)mem_alloc(offsetof(ArenaBlock, buf[capacity]));
  a->next = nullptr;
  a->allocd = 0;
  a->capacity = capacity;
  return a;
}

void arena_trash(Arena *arena) {
  ArenaBlock *a = arena->head;
  while (a != nullptr) {
    ArenaBlock *rm = a;
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

    ArenaBlock *block = arena_block_make(size);
    block->next = arena->head;

    arena->head = block;
  } while (true);

  void *ptr = &arena->head->buf[next];
  arena->head->allocd = next + size;
  return ptr;
}

String arena_bump_string(Arena *arena, String s) {
  char *cstr = (char *)arena_bump(arena, s.len + 1);
  memcpy(cstr, s.data, s.len + 1);
  return {cstr, s.len};
}
