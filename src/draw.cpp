#include "draw.h"
#include "algebra.h"
#include "app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "prelude.h"
#include "strings.h"
#include <math.h>

void renderer_setup(Renderer2D *ren) {
  ren->clear_color[0] = 0.0f;
  ren->clear_color[1] = 0.0f;
  ren->clear_color[2] = 0.0f;
  ren->clear_color[3] = 1.0f;

  ren->draw_colors[0].r = 255;
  ren->draw_colors[0].g = 255;
  ren->draw_colors[0].b = 255;
  ren->draw_colors[0].a = 255;
  ren->draw_colors_len = 1;

  ren->matrices[0] = {};
  ren->matrices[0].cols[0][0] = 1.0f;
  ren->matrices[0].cols[1][1] = 1.0f;
  ren->matrices[0].cols[2][2] = 1.0f;
  ren->matrices[0].cols[3][3] = 1.0f;
  ren->matrices_len = 1;
}

void renderer_apply_color(Renderer2D *ren) {
  Color c = ren->draw_colors[ren->draw_colors_len - 1];
  sgl_c4b(c.r, c.g, c.b, c.a);
}

bool renderer_push_color(Renderer2D *ren, Color c) {
  if (ren->draw_colors_len == array_size(ren->draw_colors)) {
    return false;
  }

  ren->draw_colors[ren->draw_colors_len++] = c;
  return true;
}

bool renderer_pop_color(Renderer2D *ren) {
  if (ren->draw_colors_len == 1) {
    return false;
  }

  ren->draw_colors_len--;
  return true;
}

bool renderer_push_matrix(Renderer2D *ren) {
  if (ren->matrices_len == array_size(ren->matrices)) {
    return false;
  }

  ren->matrices[ren->matrices_len] = ren->matrices[ren->matrices_len - 1];
  ren->matrices_len++;
  return true;
}

bool renderer_pop_matrix(Renderer2D *ren) {
  if (ren->matrices_len == 1) {
    return false;
  }

  ren->matrices_len--;
  return true;
}

Matrix4 *renderer_peek_matrix(Renderer2D *ren) {
  return &ren->matrices[ren->matrices_len - 1];
}

void renderer_translate(Renderer2D *ren, float x, float y) {
  Matrix4 m = {};
  m.cols[0][0] = 1.0f;
  m.cols[1][1] = 1.0f;
  m.cols[2][2] = 1.0f;
  m.cols[3][3] = 1.0f;

  m.cols[3][0] = x;
  m.cols[3][1] = y;

  Matrix4 *top = renderer_peek_matrix(ren);
  *top = mat4_mul_mat4(*top, m);
}

void renderer_rotate(Renderer2D *ren, float angle) {
  float s = sinf(angle);
  float c = cosf(angle);

  Matrix4 m = {};
  m.cols[0][0] = c;
  m.cols[0][1] = s;
  m.cols[1][0] = -s;
  m.cols[1][1] = c;
  m.cols[2][2] = 1.0f;
  m.cols[3][3] = 1.0f;

  Matrix4 *top = renderer_peek_matrix(ren);
  *top = mat4_mul_mat4(*top, m);
}

void renderer_scale(Renderer2D *ren, float x, float y) {
  Matrix4 m = {};
  m.cols[0][0] = x;
  m.cols[1][1] = y;
  m.cols[2][2] = 1.0f;
  m.cols[3][3] = 1.0f;

  Matrix4 *top = renderer_peek_matrix(ren);
  *top = mat4_mul_mat4(*top, m);
}

void renderer_push_quad(Renderer2D *ren, Vector4 pos, Vector4 tex) {
  Matrix4 top = *renderer_peek_matrix(ren);
  Vector4 a = vec4_mul_mat4({pos.x, pos.y, 0, 1}, top);
  Vector4 b = vec4_mul_mat4({pos.x, pos.w, 0, 1}, top);
  Vector4 c = vec4_mul_mat4({pos.z, pos.w, 0, 1}, top);
  Vector4 d = vec4_mul_mat4({pos.z, pos.y, 0, 1}, top);

  sgl_v2f_t2f(a.x, a.y, tex.x, tex.y);
  sgl_v2f_t2f(b.x, b.y, tex.x, tex.w);
  sgl_v2f_t2f(c.x, c.y, tex.z, tex.w);
  sgl_v2f_t2f(d.x, d.y, tex.z, tex.y);
}

void draw(Renderer2D *ren, Image *img, DrawDescription *desc) {
  bool ok = renderer_push_matrix(ren);
  if (!ok) {
    return;
  }

  renderer_translate(ren, desc->x, desc->y);
  renderer_rotate(ren, desc->rotation);
  renderer_scale(ren, desc->sx, desc->sy);

  sgl_enable_texture();
  sgl_texture({img->id});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (desc->u1 - desc->u0) * img->width - desc->ox;
  float y1 = (desc->v1 - desc->v0) * img->height - desc->oy;

  renderer_apply_color(ren);
  renderer_push_quad(ren, {x0, y0, x1, y1},
                     {desc->u0, desc->v0, desc->u1, desc->v1});

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw(Renderer2D *ren, SpriteRenderer *sr, DrawDescription *desc) {
  Sprite *spr = &g_app->assets[sr->sprite].sprite;
  SpriteLoop *loop = get(&spr->by_tag, sr->loop);

  bool ok = renderer_push_matrix(ren);
  if (!ok) {
    return;
  }

  renderer_translate(ren, desc->x, desc->y);
  renderer_rotate(ren, desc->rotation);
  renderer_scale(ren, desc->sx, desc->sy);

  sgl_enable_texture();
  sgl_texture({spr->img.id});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (float)spr->width - desc->ox;
  float y1 = (float)spr->height - desc->oy;

  i32 index;
  if (loop != nullptr) {
    index = loop->indices[sr->current_frame];
  } else {
    index = sr->current_frame;
  }

  SpriteFrame f = spr->frames[index];

  renderer_apply_color(ren);
  renderer_push_quad(ren, {x0, y0, x1, y1}, {f.u0, f.v0, f.u1, f.v1});

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw(Renderer2D *ren, FontFamily *font, float size, float x, float y,
          String text) {
  float start_x = x;
  y += size;
  sgl_enable_texture();

  renderer_apply_color(ren);

  for (String line : SplitLines(text)) {
    for (Rune r : UTF8(line)) {
      u32 atlas = 0;
      float xpos = x;
      float ypos = y;
      stbtt_aligned_quad q =
          font_quad(font, &atlas, &xpos, &ypos, size, rune_charcode(r));

      sgl_texture({atlas});
      sgl_begin_quads();
      renderer_push_quad(ren, {x + q.x0, y + q.y0, x + q.x1, y + q.y1},
                         {q.s0, q.t0, q.s1, q.t1});
      sgl_end();

      x = xpos;
      y = ypos;
    }

    y += size;
    x = start_x;
  }
}

void draw(Renderer2D *ren, Tilemap *tm) {
  sgl_enable_texture();
  renderer_apply_color(ren);
  for (TilemapLevel &level : tm->levels) {
    bool ok = renderer_push_matrix(ren);
    if (!ok) {
      return;
    }

    renderer_translate(ren, level.world_x, level.world_y);
    for (i32 i = level.layers.len - 1; i >= 0; i--) {
      TilemapLayer &layer = level.layers[i];
      sgl_texture({layer.image.id});
      sgl_begin_quads();
      for (TilemapTile tile : layer.tiles) {
        float x0 = tile.x;
        float y0 = tile.y;
        float x1 = tile.x + layer.grid_size;
        float y1 = tile.y + layer.grid_size;

        renderer_push_quad(ren, {x0, y0, x1, y1},
                           {tile.u0, tile.v0, tile.u1, tile.v1});
      }
      sgl_end();
    }
    renderer_pop_matrix(ren);
  }
}

void draw_filled_rect(Renderer2D *ren, RectDescription *desc) {
  bool ok = renderer_push_matrix(ren);
  if (!ok) {
    return;
  }

  renderer_translate(ren, desc->x, desc->y);
  renderer_rotate(ren, desc->rotation);
  renderer_scale(ren, desc->sx, desc->sy);

  sgl_disable_texture();
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  renderer_apply_color(ren);
  renderer_push_quad(ren, {x0, y0, x1, y1}, {0, 0, 0, 0});

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_line_rect(Renderer2D *ren, RectDescription *desc) {
  bool ok = renderer_push_matrix(ren);
  if (!ok) {
    return;
  }

  renderer_translate(ren, desc->x, desc->y);
  renderer_rotate(ren, desc->rotation);
  renderer_scale(ren, desc->sx, desc->sy);

  sgl_disable_texture();
  sgl_begin_line_strip();

  Matrix4 top = *renderer_peek_matrix(ren);
  Vector4 xy0 = vec4_mul_mat4({-desc->ox, -desc->oy, 0, 1}, top);
  Vector4 xy1 =
      vec4_mul_mat4({desc->w - desc->ox, desc->h - desc->oy, 0, 1}, top);

  float x0 = xy0.x;
  float y0 = xy0.y;
  float x1 = xy1.x;
  float y1 = xy1.y;

  renderer_apply_color(ren);
  sgl_v2f(x0, y0);
  sgl_v2f(x0, y1);
  sgl_v2f(x1, y1);
  sgl_v2f(x1, y0);
  sgl_v2f(x0, y0);

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_line_circle(Renderer2D *ren, float x, float y, float radius) {
  sgl_disable_texture();
  sgl_begin_line_strip();

  renderer_apply_color(ren);
  Matrix4 top = *renderer_peek_matrix(ren);
  constexpr float tau = MATH_PI * 2.0f;
  for (float i = 0; i <= tau + 0.001f; i += tau / 36.0f) {
    float c = cosf(i) * radius;
    float s = sinf(i) * radius;
    Vector4 pos = vec4_mul_mat4({x + c, y + s, 0, 1}, top);
    sgl_v2f(pos.x, pos.y);
  }

  sgl_end();
}

void draw_line(Renderer2D *ren, float x0, float y0, float x1, float y1) {
  sgl_disable_texture();
  sgl_begin_lines();

  renderer_apply_color(ren);

  Matrix4 top = *renderer_peek_matrix(ren);
  Vector4 xy0 = vec4_mul_mat4({x0, y0, 0, 1}, top);
  Vector4 xy1 = vec4_mul_mat4({x1, y1, 0, 1}, top);
  sgl_v2f(xy0.x, xy0.y);
  sgl_v2f(xy1.x, xy1.y);

  sgl_end();
}
