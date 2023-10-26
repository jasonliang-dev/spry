#pragma once

#include "deps/stb_truetype.h"
#include "hash_map.h"
#include "image.h"

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
};

bool font_load(FontFamily *font, Archive *ar, String filepath);
void font_load_default(FontFamily *font);
void drop(FontFamily *font);
stbtt_aligned_quad font_quad(FontFamily *font, u32 *img, float *x, float *y,
                             float size, i32 ch);
float font_width(FontFamily *font, float size, String text);
