#pragma once

#include "algebra.h"
#include "deps/lua/lua.h"
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

void renderer_reset();
void renderer_use_sampler(u32 sampler);
void renderer_get_clear_color(float *rgba);
void renderer_set_clear_color(float *rgba);
void renderer_apply_color();
bool renderer_push_color(Color c);
bool renderer_pop_color();
bool renderer_push_matrix();
bool renderer_pop_matrix();
Matrix4 renderer_peek_matrix();
void renderer_set_top_matrix(Matrix4 mat);
void renderer_translate(float x, float y);
void renderer_rotate(float angle);
void renderer_scale(float x, float y);
void renderer_push_quad(Vector4 pos, Vector4 tex);
void renderer_push_xy(float x, float y);

void draw_image(const Image *img, DrawDescription *desc);
void draw_sprite(Sprite *spr, DrawDescription *desc);
void draw_font(FontFamily *font, float size, float x, float y,
               String text);
void draw_tilemap(const Tilemap *tm);
void draw_filled_rect(RectDescription *desc);
void draw_line_rect(RectDescription *desc);
void draw_line_circle(float x, float y, float radius);
void draw_line(float x0, float y0, float x1, float y1);

DrawDescription draw_description_args(lua_State *L, i32 arg_start);
RectDescription rect_description_args(lua_State *L, i32 arg_start);
