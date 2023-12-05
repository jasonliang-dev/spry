#include "font.h"
#include "deps/sokol_gfx.h"
#include "embed/cousine_compressed.h"
#include "prelude.h"
#include "profile.h"
#include "stb_decompress.h"
#include "strings.h"
#include "vfs.h"
#include <stdio.h>

bool FontFamily::load(String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = vfs_read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }

  FontFamily f = {};
  f.ttf = contents;
  f.sb = {};
  *this = f;
  return true;
}

void FontFamily::load_default() {
  PROFILE_FUNC();

  String contents =
      stb_decompress_data(cousine_compressed_data, cousine_compressed_size);

  FontFamily f = {};
  f.ttf = contents;
  f.sb = {};
  *this = f;
}

void FontFamily::trash() {
  for (auto [k, v] : ranges) {
    v->image.trash();
  }
  sb.trash();
  ranges.trash();
  mem_free(ttf.data);
}

struct FontKey {
  float size;
  i32 ch;
};

static FontKey font_key(float size, i32 charcode) {
  FontKey fk = {};
  fk.size = size;
  fk.ch =
      (charcode / array_size(FontRange::chars)) * array_size(FontRange::chars);
  return fk;
}

static void make_font_range(FontRange *out, FontFamily *font, FontKey key) {
  PROFILE_FUNC();

  i32 width = 256;
  i32 height = 256;

  u8 *bitmap = nullptr;
  while (bitmap == nullptr) {
    PROFILE_BLOCK("try bake");

    bitmap = (u8 *)mem_alloc(width * height);
    i32 res = stbtt_BakeFontBitmap((u8 *)font->ttf.data, 0, key.size, bitmap,
                                   width, height, key.ch,
                                   array_size(out->chars), out->chars);
    if (res < 0) {
      mem_free(bitmap);
      bitmap = nullptr;
      width *= 2;
      height *= 2;
    }
  }
  defer(mem_free(bitmap));

  u8 *image = (u8 *)mem_alloc(width * height * 4);
  defer(mem_free(image));

  {
    PROFILE_BLOCK("convert rgba");

    for (i32 i = 0; i < width * height * 4; i += 4) {
      image[i + 0] = 255;
      image[i + 1] = 255;
      image[i + 2] = 255;
      image[i + 3] = bitmap[i / 4];
    }
  }

  u32 id = 0;
  {
    PROFILE_BLOCK("make image");

    sg_image_desc sg_image = {};
    sg_image.width = width;
    sg_image.height = height;
    sg_image.data.subimage[0][0].ptr = image;
    sg_image.data.subimage[0][0].size = width * height * 4;

    id = sg_make_image(sg_image).id;
  }

  out->image.id = id;
  out->image.width = width;
  out->image.height = height;

  printf("created font range with id %d\n", id);
}

static FontRange *get_range(FontFamily *font, FontKey key) {
  u64 hash = *(u64 *)&key;
  FontRange *range = font->ranges.get(hash);
  if (range == nullptr) {
    range = &font->ranges[hash];
    make_font_range(range, font, key);
  }

  return range;
}

stbtt_aligned_quad FontFamily::quad(u32 *img, float *x, float *y, float size,
                                    i32 ch) {
  FontRange *range = get_range(this, font_key(size, ch));
  assert(range != nullptr);

  ch = ch % array_size(FontRange::chars);

  float xpos = 0;
  float ypos = 0;
  stbtt_aligned_quad q = {};
  stbtt_GetBakedQuad(range->chars, (i32)range->image.width,
                     (i32)range->image.height, ch, &xpos, &ypos, &q, 1);

  stbtt_bakedchar *baked = range->chars + ch;
  *img = range->image.id;
  *x = *x + baked->xadvance;
  return q;
}

float FontFamily::width(float size, String text) {
  float width = 0;
  for (Rune r : UTF8(text)) {
    u32 code = r.charcode();
    FontRange *range = get_range(this, font_key(size, code));
    assert(range != nullptr);

    const stbtt_bakedchar *baked =
        range->chars + (code % array_size(FontRange::chars));
    width += baked->xadvance;
  }
  return width;
}
