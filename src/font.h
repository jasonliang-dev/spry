#pragma once

#include "deps/stb_truetype.h"
#include "hash_map.h"
#include "image.h"
#include "strings.h"

struct FontRange {
  stbtt_bakedchar chars[256];
  Image image;
};

struct FontQuad {
  stbtt_aligned_quad quad;
};

struct FontFamily {
  String ttf;
  HashMap<FontRange> ranges;
  StringBuilder sb;

  bool load(String filepath);
  void load_default();
  void trash();

  stbtt_aligned_quad quad(u32 *img, float *x, float *y, float size, i32 ch);
  float width(float size, String text);
};