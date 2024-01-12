#pragma once

#include "prelude.h"

struct Image {
  u32 id;
  i32 width;
  i32 height;
  bool has_mips;

  bool load(String filepath, bool generate_mips);
  void trash();
};