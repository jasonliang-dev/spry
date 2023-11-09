#pragma once

#include "prelude.h"

struct ArenaNode;
struct Arena {
  ArenaNode *head;
};

void arena_trash(Arena *arena);
void *arena_bump(Arena *arena, u64 size);
void *arena_rebump(Arena *arena, void *ptr, u64 old, u64 size);
String arena_bump_string(Arena *arena, String s);
