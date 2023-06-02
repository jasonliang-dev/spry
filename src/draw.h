#pragma once

#include "font.h"
#include "image.h"
#include "sprite.h"
#include "tilemap.h"

struct DrawDescription {
  float x;
  float y;
  float rotation;

  float sx; // scale
  float sy;

  float ox; // origin
  float oy;

  float u0; // uv coords
  float v0;
  float u1;
  float v1;
};

struct RectDescription {
  float x;
  float y;
  float w;
  float h;

  float rotation;

  float sx; // scale
  float sy;

  float ox; // origin
  float oy;
};

struct Color {
  u8 r, g, b, a;
};

void draw(Image *img, DrawDescription *desc, Color c);
void draw(SpriteRenderer *sr, DrawDescription *desc, Color c);
void draw(FontFamily *font, u64 size, float x, float y, String text, Color c);
void draw(Tilemap *tm, Color c);
void draw_filled_rect(RectDescription *desc, Color c);
void draw_line_rect(RectDescription *desc, Color c);
void draw_line_circle(float x, float y, float radius, Color c);
void draw_line(float x0, float y0, float x1, float y1, Color c);
