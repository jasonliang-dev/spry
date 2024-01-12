#pragma once

#include "hash_map.h"
#include "image.h"

struct AtlasImage {
  float u0, v0, u1, v1;
  float width;
  float height;
  Image img;
};

struct Atlas {
  HashMap<AtlasImage> by_name;
  Image img;

  bool load(String filepath, bool generate_mips);
  void trash();
  AtlasImage *get(String name);
};
