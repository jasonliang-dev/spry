#include "draw.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "prelude.h"
#include "strings.h"
#include <math.h>

void draw(Image *img, DrawDescription *desc, Color c) {
  sgl_push_matrix();
  sgl_translate(desc->x, desc->y, 0);
  sgl_rotate(desc->rotation, 0, 0, 1);
  sgl_scale(desc->sx, desc->sy, 1);

  sgl_enable_texture();
  sgl_texture({img->id});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (desc->u1 - desc->u0) * img->width - desc->ox;
  float y1 = (desc->v1 - desc->v0) * img->height - desc->oy;

  sgl_c4b(c.r, c.g, c.b, c.a);
  sgl_v2f_t2f(x0, y0, desc->u0, desc->v0);
  sgl_v2f_t2f(x0, y1, desc->u0, desc->v1);
  sgl_v2f_t2f(x1, y1, desc->u1, desc->v1);
  sgl_v2f_t2f(x1, y0, desc->u1, desc->v0);

  sgl_end();
  sgl_pop_matrix();
}

void draw(SpriteRenderer *sr, DrawDescription *desc, Color c) {
  Sprite *spr = sr->sprite;

  sgl_push_matrix();
  sgl_translate(desc->x, desc->y, 0);
  sgl_rotate(desc->rotation, 0, 0, 1);
  sgl_scale(desc->sx, desc->sy, 1);

  sgl_enable_texture();
  sgl_texture({spr->img.id});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (float)spr->width - desc->ox;
  float y1 = (float)spr->height - desc->oy;

  i32 index;
  if (sr->loop) {
    index = sr->loop->indices[sr->current_frame];
  } else {
    index = sr->current_frame;
  }

  SpriteFrame f = spr->frames[index];

  sgl_c4b(c.r, c.g, c.b, c.a);
  sgl_v2f_t2f(x0, y0, f.u0, f.v0);
  sgl_v2f_t2f(x0, y1, f.u0, f.v1);
  sgl_v2f_t2f(x1, y1, f.u1, f.v1);
  sgl_v2f_t2f(x1, y0, f.u1, f.v0);

  sgl_end();
  sgl_pop_matrix();
}

void draw(FontFamily *font, float size, float x, float y, String text,
          Color c) {
  y += size;
  sgl_enable_texture();
  sgl_c4b(c.r, c.g, c.b, c.a);

  for (Rune r : UTF8(text)) {
    u32 atlas = 0;
    stbtt_aligned_quad q =
        font_quad(font, &atlas, &x, &y, size, rune_charcode(r));

    sgl_texture({atlas});
    sgl_begin_quads();
    sgl_v2f_t2f(x + q.x0, y + q.y0, q.s0, q.t0);
    sgl_v2f_t2f(x + q.x0, y + q.y1, q.s0, q.t1);
    sgl_v2f_t2f(x + q.x1, y + q.y1, q.s1, q.t1);
    sgl_v2f_t2f(x + q.x1, y + q.y0, q.s1, q.t0);
    sgl_end();
  }
}

void draw(Tilemap *tm, Color c) {
  sgl_enable_texture();
  sgl_c4b(c.r, c.g, c.b, c.a);
  for (TilemapLevel level : tm->levels) {
    sgl_push_matrix();
    sgl_translate(level.world_x, level.world_y, 0);
    for (u64 i = level.layers.len; i > 0; i--) {
      TilemapLayer &layer = level.layers[i - 1];
      sgl_texture({layer.image.id});
      sgl_begin_quads();
      for (TilemapTile tile : layer.tiles) {
        float x0 = tile.x;
        float y0 = tile.y;
        float x1 = tile.x + layer.grid_size;
        float y1 = tile.y + layer.grid_size;

        sgl_v2f_t2f(x0, y0, tile.u0, tile.v0);
        sgl_v2f_t2f(x0, y1, tile.u0, tile.v1);
        sgl_v2f_t2f(x1, y1, tile.u1, tile.v1);
        sgl_v2f_t2f(x1, y0, tile.u1, tile.v0);
      }
      sgl_end();
    }
    sgl_pop_matrix();
  }
}

void draw_filled_rect(RectDescription *desc, Color c) {
  sgl_push_matrix();
  sgl_translate(desc->x, desc->y, 0);
  sgl_rotate(desc->rotation, 0, 0, 1);
  sgl_scale(desc->sx, desc->sy, 1);

  sgl_disable_texture();
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  sgl_c4b(c.r, c.g, c.b, c.a);
  sgl_v2f(x0, y0);
  sgl_v2f(x0, y1);
  sgl_v2f(x1, y1);
  sgl_v2f(x1, y0);

  sgl_end();
  sgl_pop_matrix();
}

void draw_line_rect(RectDescription *desc, Color c) {
  sgl_push_matrix();
  sgl_translate(desc->x, desc->y, 0);
  sgl_rotate(desc->rotation, 0, 0, 1);
  sgl_scale(desc->sx, desc->sy, 1);

  sgl_disable_texture();
  sgl_begin_line_strip();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  sgl_c4b(c.r, c.g, c.b, c.a);
  sgl_v2f(x0, y0);
  sgl_v2f(x0, y1);
  sgl_v2f(x1, y1);
  sgl_v2f(x1, y0);
  sgl_v2f(x0, y0);

  sgl_end();
  sgl_pop_matrix();
}

void draw_line_circle(float x, float y, float radius, Color c) {
  sgl_disable_texture();
  sgl_begin_line_strip();

  sgl_c4b(c.r, c.g, c.b, c.a);
  constexpr float tau = MATH_PI * 2.0f;
  for (float i = 0; i <= tau + 0.001f; i += tau / 36.0f) {
    float c = cosf(i) * radius;
    float s = sinf(i) * radius;
    sgl_v2f(x + c, y + s);
  }

  sgl_end();
}

void draw_line(float x0, float y0, float x1, float y1, Color c) {
  sgl_disable_texture();
  sgl_begin_lines();

  sgl_c4b(c.r, c.g, c.b, c.a);
  sgl_v2f(x0, y0);
  sgl_v2f(x1, y1);

  sgl_end();
}
