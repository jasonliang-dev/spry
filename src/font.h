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
};

bool font_load(FontFamily *font, String filepath);
void font_load_default(FontFamily *font);
void font_trash(FontFamily *font);
stbtt_aligned_quad font_quad(FontFamily *font, u32 *img, float *x, float *y,
                             float size, i32 ch);
float font_width(FontFamily *font, float size, String text);
