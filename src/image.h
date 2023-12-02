#pragma once

#include "prelude.h"

struct Image {
  u32 id;
  i32 width;
  i32 height;

  bool load(String filepath);
  void trash();
};