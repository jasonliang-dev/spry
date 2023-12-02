#pragma once

#include "prelude.h"

struct ArenaNode;
struct Arena {
  ArenaNode *head;

  void trash();
  void *bump(u64 size);
  void *rebump(void *ptr, u64 old, u64 size);
  String bump_string(String s);
};
