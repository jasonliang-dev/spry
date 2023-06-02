#pragma once

#include "prelude.h"

struct Scanner {
  char *data;
  u64 len;
  u64 pos;
  u64 end;
};

Scanner make_scanner(String str);
String next_string(Scanner *s);
i32 next_int(Scanner *s);
