#pragma once

#include "prelude.h"

struct Image {
  u32 id;
  i32 width;
  i32 height;
};

bool image_load(Image *image, String filepath);
void image_trash(Image *image);