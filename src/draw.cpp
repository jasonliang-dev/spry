#include "draw.h"
#include "algebra.h"
#include "app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "prelude.h"
#include "profile.h"
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

Matrix4 renderer_peek_matrix(Renderer2D *ren) {
  return ren->matrices[ren->matrices_len - 1];
}

void renderer_set_top_matrix(Renderer2D *ren, Matrix4 mat) {
  ren->matrices[ren->matrices_len - 1] = mat;
}

void renderer_translate(Renderer2D *ren, float x, float y) {
  Matrix4 top = renderer_peek_matrix(ren);

#ifdef SSE_AVAILABLE
  __m128 xx = _mm_mul_ps(_mm_set1_ps(x), top.sse[0]);
  __m128 yy = _mm_mul_ps(_mm_set1_ps(y), top.sse[1]);
  top.sse[3] =
      _mm_add_ps(_mm_add_ps(xx, yy), _mm_add_ps(top.sse[2], top.sse[3]));
#else
  for (i32 i = 0; i < 4; i++) {
    top.cols[3][i] = x * top.cols[0][i] + y * top.cols[1][i] + top.cols[2][i] +
                     top.cols[3][i];
  }
#endif

  renderer_set_top_matrix(ren, top);
}

void renderer_rotate(Renderer2D *ren, float angle) {
  Matrix4 top = renderer_peek_matrix(ren);

#ifdef SSE_AVAILABLE
  __m128 v0 = top.sse[0];
  __m128 v1 = top.sse[1];
  __m128 c = _mm_set1_ps(cos(-angle));
  __m128 s = _mm_set1_ps(sin(-angle));

  top.sse[0] = _mm_sub_ps(_mm_mul_ps(c, v0), _mm_mul_ps(s, v1));
  top.sse[1] = _mm_add_ps(_mm_mul_ps(s, v0), _mm_mul_ps(c, v1));
#else
  float c = cos(-angle);
  float s = sin(-angle);

  for (i32 i = 0; i < 4; i++) {
    float x = c * top.cols[0][i] - s * top.cols[1][i];
    float y = s * top.cols[0][i] + c * top.cols[1][i];
    top.cols[0][i] = x;
    top.cols[1][i] = y;
  }
#endif

  renderer_set_top_matrix(ren, top);
}

void renderer_scale(Renderer2D *ren, float x, float y) {
  Matrix4 top = renderer_peek_matrix(ren);

#ifdef SSE_AVAILABLE
  top.sse[0] = _mm_mul_ps(top.sse[0], _mm_set1_ps(x));
  top.sse[1] = _mm_mul_ps(top.sse[1], _mm_set1_ps(y));
#else
  for (i32 i = 0; i < 4; i++) {
    top.cols[0][i] *= x;
    top.cols[1][i] *= y;
  }
#endif

  renderer_set_top_matrix(ren, top);
}

void renderer_push_quad(Renderer2D *ren, Vector4 pos, Vector4 tex) {
  Matrix4 top = renderer_peek_matrix(ren);
  Vector4 a = vec4_mul_mat4(vec4_xy(pos.x, pos.y), top);
  Vector4 b = vec4_mul_mat4(vec4_xy(pos.x, pos.w), top);
  Vector4 c = vec4_mul_mat4(vec4_xy(pos.z, pos.w), top);
  Vector4 d = vec4_mul_mat4(vec4_xy(pos.z, pos.y), top);

  sgl_v2f_t2f(a.x, a.y, tex.x, tex.y);
  sgl_v2f_t2f(b.x, b.y, tex.x, tex.w);
  sgl_v2f_t2f(c.x, c.y, tex.z, tex.w);
  sgl_v2f_t2f(d.x, d.y, tex.z, tex.y);
}

void renderer_push_xy(Renderer2D *ren, float x, float y) {
  Matrix4 top = renderer_peek_matrix(ren);
  Vector4 v = vec4_mul_mat4(vec4_xy(x, y), top);
  sgl_v2f(v.x, v.y);
}

void draw_image(Renderer2D *ren, Image *img, DrawDescription *desc) {
  PROFILE_FUNC();

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
  renderer_push_quad(ren, vec4(x0, y0, x1, y1),
                     vec4(desc->u0, desc->v0, desc->u1, desc->v1));

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_sprite(Renderer2D *ren, SpriteRenderer *sr, DrawDescription *desc) {
  PROFILE_FUNC();

  Sprite *spr = &g_app->assets[sr->sprite].sprite;
  SpriteLoop *loop = hashmap_get(&spr->by_tag, sr->loop);

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
  renderer_push_quad(ren, vec4(x0, y0, x1, y1), vec4(f.u0, f.v0, f.u1, f.v1));

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_font(Renderer2D *ren, FontFamily *font, float size, float x, float y,
               String text) {
  PROFILE_FUNC();

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
      renderer_push_quad(ren, vec4(x + q.x0, y + q.y0, x + q.x1, y + q.y1),
                         vec4(q.s0, q.t0, q.s1, q.t1));
      sgl_end();

      x = xpos;
      y = ypos;
    }

    y += size;
    x = start_x;
  }
}

void draw_tilemap(Renderer2D *ren, Tilemap *tm) {
  PROFILE_FUNC();

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

        renderer_push_quad(ren, vec4(x0, y0, x1, y1),
                           vec4(tile.u0, tile.v0, tile.u1, tile.v1));
      }
      sgl_end();
    }
    renderer_pop_matrix(ren);
  }
}

void draw_filled_rect(Renderer2D *ren, RectDescription *desc) {
  PROFILE_FUNC();

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
  renderer_push_quad(ren, vec4(x0, y0, x1, y1), vec4(0, 0, 0, 0));

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_line_rect(Renderer2D *ren, RectDescription *desc) {
  PROFILE_FUNC();

  bool ok = renderer_push_matrix(ren);
  if (!ok) {
    return;
  }

  renderer_translate(ren, desc->x, desc->y);
  renderer_rotate(ren, desc->rotation);
  renderer_scale(ren, desc->sx, desc->sy);

  sgl_disable_texture();
  sgl_begin_line_strip();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  Matrix4 top = renderer_peek_matrix(ren);
  Vector4 a = vec4_mul_mat4(vec4_xy(x0, y0), top);
  Vector4 b = vec4_mul_mat4(vec4_xy(x0, y1), top);
  Vector4 c = vec4_mul_mat4(vec4_xy(x1, y1), top);
  Vector4 d = vec4_mul_mat4(vec4_xy(x1, y0), top);

  renderer_apply_color(ren);
  sgl_v2f(a.x, a.y);
  sgl_v2f(b.x, b.y);
  sgl_v2f(c.x, c.y);
  sgl_v2f(d.x, d.y);
  sgl_v2f(a.x, a.y);

  sgl_end();
  renderer_pop_matrix(ren);
}

void draw_line_circle(Renderer2D *ren, float x, float y, float radius) {
  PROFILE_FUNC();

  sgl_disable_texture();
  sgl_begin_line_strip();

  renderer_apply_color(ren);
  constexpr float tau = MATH_PI * 2.0f;
  for (float i = 0; i <= tau + 0.001f; i += tau / 36.0f) {
    float c = cosf(i) * radius;
    float s = sinf(i) * radius;
    renderer_push_xy(ren, x + c, y + s);
  }

  sgl_end();
}

void draw_line(Renderer2D *ren, float x0, float y0, float x1, float y1) {
  PROFILE_FUNC();

  sgl_disable_texture();
  sgl_begin_lines();

  renderer_apply_color(ren);

  renderer_push_xy(ren, x0, y0);
  renderer_push_xy(ren, x1, y1);

  sgl_end();
}
