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
};

bool atlas_load(Atlas *atlas, String filepath);
void atlas_trash(Atlas *atlas);
AtlasImage *atlas_get(Atlas *atlas, String name);