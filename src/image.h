#pragma once

#include "archive.h"
#include "hash_map.h"

struct Image {
  u32 id;
  i32 width;
  i32 height;
};

bool image_load(Image *image, Archive *ar, String filepath);
void drop(Image *image);