#pragma once

#include "prelude.h"

struct ArenaBlock;
struct Arena {
  ArenaBlock *head;
};

void arena_trash(Arena *arena);
void *arena_bump(Arena *arena, u64 size);
String arena_bump_string(Arena *arena, String s);
