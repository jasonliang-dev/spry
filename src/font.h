#pragma once

#include "deps/stb_truetype.h"
#include "hash_map.h"
#include "image.h"

struct FontRange {
  stbtt_bakedchar chars[128];
  Image image;
};

struct FontFamily {
  String ttf;
  HashMap<FontRange> ranges;
  FontRange *current_range;
};

bool font_load(FontFamily *font, Archive *ar, String filepath);
void font_load_default(FontFamily *font);
void drop(FontFamily *font);
void font_begin(FontFamily *font, u64 size);
void font_end(FontFamily *font);
stbtt_aligned_quad font_quad(FontFamily *font, i32 ch);
float font_width(FontFamily *font, String text);
