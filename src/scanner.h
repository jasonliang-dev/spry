#pragma once

#include "prelude.h"

struct Scanner {
  char *data;
  u64 len;
  u64 pos;
  u64 end;

  Scanner(String str);
  String next_string();
  i32 next_int();
};
