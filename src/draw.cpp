#include "draw.h"
#include "algebra.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "font.h"
#include "prelude.h"
#include "profile.h"
#include "scanner.h"
#include "strings.h"
#include <math.h>

extern "C" {
#include <lauxlib.h>
}

struct Renderer2D {
  Matrix4 matrices[32];
  u64 matrices_len;

  float clear_color[4];
  Color draw_colors[32];
  u64 draw_colors_len;

  u32 sampler;
};

static Renderer2D g_renderer;

void renderer_reset() {
  g_renderer.clear_color[0] = 0.0f;
  g_renderer.clear_color[1] = 0.0f;
  g_renderer.clear_color[2] = 0.0f;
  g_renderer.clear_color[3] = 1.0f;

  g_renderer.draw_colors[0].r = 255;
  g_renderer.draw_colors[0].g = 255;
  g_renderer.draw_colors[0].b = 255;
  g_renderer.draw_colors[0].a = 255;
  g_renderer.draw_colors_len = 1;

  g_renderer.matrices[0] = {};
  g_renderer.matrices[0].cols[0][0] = 1.0f;
  g_renderer.matrices[0].cols[1][1] = 1.0f;
  g_renderer.matrices[0].cols[2][2] = 1.0f;
  g_renderer.matrices[0].cols[3][3] = 1.0f;
  g_renderer.matrices_len = 1;

  g_renderer.sampler = SG_INVALID_ID;
}

void renderer_use_sampler(u32 sampler) { g_renderer.sampler = sampler; }

void renderer_get_clear_color(float *rgba) {
  memcpy(rgba, g_renderer.clear_color, sizeof(float) * 4);
}

void renderer_set_clear_color(float *rgba) {
  memcpy(g_renderer.clear_color, rgba, sizeof(float) * 4);
}

void renderer_apply_color() {
  Color c = g_renderer.draw_colors[g_renderer.draw_colors_len - 1];
  sgl_c4b(c.r, c.g, c.b, c.a);
}

bool renderer_push_color(Color c) {
  if (g_renderer.draw_colors_len == array_size(g_renderer.draw_colors)) {
    return false;
  }

  g_renderer.draw_colors[g_renderer.draw_colors_len++] = c;
  return true;
}

bool renderer_pop_color() {
  if (g_renderer.draw_colors_len == 1) {
    return false;
  }

  g_renderer.draw_colors_len--;
  return true;
}

bool renderer_push_matrix() {
  if (g_renderer.matrices_len == array_size(g_renderer.matrices)) {
    return false;
  }

  g_renderer.matrices[g_renderer.matrices_len] =
      g_renderer.matrices[g_renderer.matrices_len - 1];
  g_renderer.matrices_len++;
  return true;
}

bool renderer_pop_matrix() {
  if (g_renderer.matrices_len == 1) {
    return false;
  }

  g_renderer.matrices_len--;
  return true;
}

Matrix4 renderer_peek_matrix() {
  return g_renderer.matrices[g_renderer.matrices_len - 1];
}

void renderer_set_top_matrix(Matrix4 mat) {
  g_renderer.matrices[g_renderer.matrices_len - 1] = mat;
}

void renderer_translate(float x, float y) {
  Matrix4 top = renderer_peek_matrix();

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

  renderer_set_top_matrix(top);
}

void renderer_rotate(float angle) {
  Matrix4 top = renderer_peek_matrix();

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

  renderer_set_top_matrix(top);
}

void renderer_scale(float x, float y) {
  Matrix4 top = renderer_peek_matrix();

#ifdef SSE_AVAILABLE
  top.sse[0] = _mm_mul_ps(top.sse[0], _mm_set1_ps(x));
  top.sse[1] = _mm_mul_ps(top.sse[1], _mm_set1_ps(y));
#else
  for (i32 i = 0; i < 4; i++) {
    top.cols[0][i] *= x;
    top.cols[1][i] *= y;
  }
#endif

  renderer_set_top_matrix(top);
}

void renderer_push_quad(Vector4 pos, Vector4 tex) {
  Matrix4 top = renderer_peek_matrix();
  Vector4 a = vec4_mul_mat4(vec4_xy(pos.x, pos.y), top);
  Vector4 b = vec4_mul_mat4(vec4_xy(pos.x, pos.w), top);
  Vector4 c = vec4_mul_mat4(vec4_xy(pos.z, pos.w), top);
  Vector4 d = vec4_mul_mat4(vec4_xy(pos.z, pos.y), top);

  sgl_v2f_t2f(a.x, a.y, tex.x, tex.y);
  sgl_v2f_t2f(b.x, b.y, tex.x, tex.w);
  sgl_v2f_t2f(c.x, c.y, tex.z, tex.w);
  sgl_v2f_t2f(d.x, d.y, tex.z, tex.y);
}

void renderer_push_xy(float x, float y) {
  Matrix4 top = renderer_peek_matrix();
  Vector4 v = vec4_mul_mat4(vec4_xy(x, y), top);
  sgl_v2f(v.x, v.y);
}

void draw_image(const Image *img, DrawDescription *desc) {
  bool ok = renderer_push_matrix();
  if (!ok) {
    return;
  }

  renderer_translate(desc->x, desc->y);
  renderer_rotate(desc->rotation);
  renderer_scale(desc->sx, desc->sy);

  sgl_enable_texture();
  sgl_texture({img->id}, {g_renderer.sampler});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (desc->u1 - desc->u0) * img->width - desc->ox;
  float y1 = (desc->v1 - desc->v0) * img->height - desc->oy;

  renderer_apply_color();
  renderer_push_quad(vec4(x0, y0, x1, y1),
                     vec4(desc->u0, desc->v0, desc->u1, desc->v1));

  sgl_end();
  renderer_pop_matrix();
}

void draw_sprite(Sprite *spr, DrawDescription *desc) {
  bool ok = false;

  ok = renderer_push_matrix();
  if (!ok) {
    return;
  }

  SpriteView view = {};
  ok = view.make(spr);
  if (!ok) {
    return;
  }

  renderer_translate(desc->x, desc->y);
  renderer_rotate(desc->rotation);
  renderer_scale(desc->sx, desc->sy);

  sgl_enable_texture();
  sgl_texture({view.data.img.id}, {g_renderer.sampler});
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = (float)view.data.width - desc->ox;
  float y1 = (float)view.data.height - desc->oy;

  SpriteFrame f = view.data.frames[view.frame()];

  renderer_apply_color();
  renderer_push_quad(vec4(x0, y0, x1, y1), vec4(f.u0, f.v0, f.u1, f.v1));

  sgl_end();
  renderer_pop_matrix();
}

static void draw_font_line(FontFamily *font, float size, float *start_x,
                           float *start_y, String line) {
  float x = *start_x;
  float y = *start_y;
  for (Rune r : UTF8(line)) {
    u32 atlas = 0;
    float xx = x;
    float yy = y;
    stbtt_aligned_quad q = font->quad(&atlas, &xx, &yy, size, rune_charcode(r));

    sgl_texture({atlas}, {g_renderer.sampler});
    sgl_begin_quads();
    renderer_push_quad(vec4(x + q.x0, y + q.y0, x + q.x1, y + q.y1),
                       vec4(q.s0, q.t0, q.s1, q.t1));
    sgl_end();

    x = xx;
    y = yy;
  }

  *start_y += size;
}

float draw_font(FontFamily *font, float size, float x, float y, String text) {
  PROFILE_FUNC();

  y += size;
  sgl_enable_texture();
  renderer_apply_color();

  for (String line : SplitLines(text)) {
    draw_font_line(font, size, &x, &y, line);
  }

  return y - size;
}

float draw_font_wrapped(FontFamily *font, float size, float x, float y,
                        String text, float limit) {
  PROFILE_FUNC();

  y += size;
  sgl_enable_texture();
  renderer_apply_color();

  for (String line : SplitLines(text)) {
    font->sb.clear();
    Scanner scan = line;

    for (String word = scan.next_string(); word != "";
         word = scan.next_string()) {

      font->sb << word;

      float width = font->width(size, font->sb);
      if (width < limit) {
        font->sb << " ";
        continue;
      }

      font->sb.len -= word.len;
      font->sb.data[font->sb.len] = '\0';

      draw_font_line(font, size, &x, &y, font->sb);

      font->sb.clear();
      font->sb << word << " ";
    }

    draw_font_line(font, size, &x, &y, font->sb);
  }

  return y - size;
}

void draw_tilemap(const Tilemap *tm) {
  PROFILE_FUNC();

  sgl_enable_texture();
  renderer_apply_color();
  for (const TilemapLevel &level : tm->levels) {
    bool ok = renderer_push_matrix();
    if (!ok) {
      return;
    }

    renderer_translate(level.world_x, level.world_y);
    for (i32 i = level.layers.len - 1; i >= 0; i--) {
      const TilemapLayer &layer = level.layers[i];
      sgl_texture({layer.image.id}, {g_renderer.sampler});
      sgl_begin_quads();
      for (Tile tile : layer.tiles) {
        float x0 = tile.x;
        float y0 = tile.y;
        float x1 = tile.x + layer.grid_size;
        float y1 = tile.y + layer.grid_size;

        renderer_push_quad(vec4(x0, y0, x1, y1),
                           vec4(tile.u0, tile.v0, tile.u1, tile.v1));
      }
      sgl_end();
    }
    renderer_pop_matrix();
  }
}

void draw_filled_rect(RectDescription *desc) {
  PROFILE_FUNC();

  bool ok = renderer_push_matrix();
  if (!ok) {
    return;
  }

  renderer_translate(desc->x, desc->y);
  renderer_rotate(desc->rotation);
  renderer_scale(desc->sx, desc->sy);

  sgl_disable_texture();
  sgl_begin_quads();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  renderer_apply_color();
  renderer_push_quad(vec4(x0, y0, x1, y1), vec4(0, 0, 0, 0));

  sgl_end();
  renderer_pop_matrix();
}

void draw_line_rect(RectDescription *desc) {
  PROFILE_FUNC();

  bool ok = renderer_push_matrix();
  if (!ok) {
    return;
  }

  renderer_translate(desc->x, desc->y);
  renderer_rotate(desc->rotation);
  renderer_scale(desc->sx, desc->sy);

  sgl_disable_texture();
  sgl_begin_line_strip();

  float x0 = -desc->ox;
  float y0 = -desc->oy;
  float x1 = desc->w - desc->ox;
  float y1 = desc->h - desc->oy;

  Matrix4 top = renderer_peek_matrix();
  Vector4 a = vec4_mul_mat4(vec4_xy(x0, y0), top);
  Vector4 b = vec4_mul_mat4(vec4_xy(x0, y1), top);
  Vector4 c = vec4_mul_mat4(vec4_xy(x1, y1), top);
  Vector4 d = vec4_mul_mat4(vec4_xy(x1, y0), top);

  renderer_apply_color();
  sgl_v2f(a.x, a.y);
  sgl_v2f(b.x, b.y);
  sgl_v2f(c.x, c.y);
  sgl_v2f(d.x, d.y);
  sgl_v2f(a.x, a.y);

  sgl_end();
  renderer_pop_matrix();
}

void draw_line_circle(float x, float y, float radius) {
  PROFILE_FUNC();

  sgl_disable_texture();
  sgl_begin_line_strip();

  renderer_apply_color();
  constexpr float tau = MATH_PI * 2.0f;
  for (float i = 0; i <= tau + 0.001f; i += tau / 36.0f) {
    float c = cosf(i) * radius;
    float s = sinf(i) * radius;
    renderer_push_xy(x + c, y + s);
  }

  sgl_end();
}

void draw_line(float x0, float y0, float x1, float y1) {
  PROFILE_FUNC();

  sgl_disable_texture();
  sgl_begin_lines();

  renderer_apply_color();

  renderer_push_xy(x0, y0);
  renderer_push_xy(x1, y1);

  sgl_end();
}

DrawDescription draw_description_args(lua_State *L, i32 arg_start) {
  DrawDescription dd;

  dd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
  dd.y = (float)luaL_optnumber(L, arg_start + 1, 0);

  dd.rotation = (float)luaL_optnumber(L, arg_start + 2, 0);

  dd.sx = (float)luaL_optnumber(L, arg_start + 3, 1);
  dd.sy = (float)luaL_optnumber(L, arg_start + 4, 1);

  dd.ox = (float)luaL_optnumber(L, arg_start + 5, 0);
  dd.oy = (float)luaL_optnumber(L, arg_start + 6, 0);

  dd.u0 = (float)luaL_optnumber(L, arg_start + 7, 0);
  dd.v0 = (float)luaL_optnumber(L, arg_start + 8, 0);
  dd.u1 = (float)luaL_optnumber(L, arg_start + 9, 1);
  dd.v1 = (float)luaL_optnumber(L, arg_start + 10, 1);

  return dd;
}

RectDescription rect_description_args(lua_State *L, i32 arg_start) {
  RectDescription rd;

  rd.x = (float)luaL_optnumber(L, arg_start + 0, 0);
  rd.y = (float)luaL_optnumber(L, arg_start + 1, 0);
  rd.w = (float)luaL_optnumber(L, arg_start + 2, 0);
  rd.h = (float)luaL_optnumber(L, arg_start + 3, 0);

  rd.rotation = (float)luaL_optnumber(L, arg_start + 4, 0);

  rd.sx = (float)luaL_optnumber(L, arg_start + 5, 1);
  rd.sy = (float)luaL_optnumber(L, arg_start + 6, 1);

  rd.ox = (float)luaL_optnumber(L, arg_start + 7, 0);
  rd.oy = (float)luaL_optnumber(L, arg_start + 8, 0);

  return rd;
}
