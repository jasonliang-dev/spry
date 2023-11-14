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

struct Renderer2D {
  Matrix4 matrices[32];
  u64 matrices_len;

  float clear_color[4];
  Color draw_colors[32];
  u64 draw_colors_len;

  u32 sampler;
};

void renderer_setup(Renderer2D *ren);
void renderer_apply_color(Renderer2D *ren);
bool renderer_push_color(Renderer2D *ren, Color c);
bool renderer_pop_color(Renderer2D *ren);
bool renderer_push_matrix(Renderer2D *ren);
bool renderer_pop_matrix(Renderer2D *ren);
Matrix4 renderer_peek_matrix(Renderer2D *ren);
void renderer_set_top_matrix(Renderer2D *ren, Matrix4 mat);
void renderer_translate(Renderer2D *ren, float x, float y);
void renderer_rotate(Renderer2D *ren, float angle);
void renderer_scale(Renderer2D *ren, float x, float y);
void renderer_push_quad(Renderer2D *ren, Vector4 pos, Vector4 tex);
void renderer_push_xy(Renderer2D *ren, float x, float y);

void draw_image(Renderer2D *ren, const Image *img, DrawDescription *desc);
void draw_sprite(Renderer2D *ren, Sprite *spr, DrawDescription *desc);
void draw_font(Renderer2D *ren, FontFamily *font, float size, float x, float y,
               String text);
void draw_tilemap(Renderer2D *ren, const Tilemap *tm);
void draw_filled_rect(Renderer2D *ren, RectDescription *desc);
void draw_line_rect(Renderer2D *ren, RectDescription *desc);
void draw_line_circle(Renderer2D *ren, float x, float y, float radius);
void draw_line(Renderer2D *ren, float x0, float y0, float x1, float y1);

DrawDescription draw_description_args(lua_State *L, i32 arg_start);
RectDescription rect_description_args(lua_State *L, i32 arg_start);
