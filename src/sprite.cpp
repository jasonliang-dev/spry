#include "sprite.h"
#include "app.h"
#include "deps/cute_aseprite.h"
#include "deps/sokol_gfx.h"

bool sprite_load(Sprite *spr, Archive *ar, String filepath) {
  String contents;
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  ase_t *ase =
      cute_aseprite_load_from_memory(contents.data, (i32)contents.len, nullptr);
  defer(cute_aseprite_free(ase));

  i32 rect = ase->w * ase->h * 4;

  Array<SpriteFrame> frames = {};
  reserve(&frames, ase->frame_count);

  Array<char> pixels = {};
  reserve(&pixels, ase->frame_count * rect);
  defer(drop(&pixels));

  for (i32 i = 0; i < ase->frame_count; i++) {
    ase_frame_t &frame = ase->frames[i];

    SpriteFrame sf = {};
    sf.duration = frame.duration_milliseconds;

    sf.u0 = 0;
    sf.v0 = (float)i / ase->frame_count;
    sf.u1 = 1;
    sf.v1 = (float)(i + 1) / ase->frame_count;

    push(&frames, sf);
    memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
  }

  sg_image_desc desc = {};
  desc.width = ase->w;
  desc.height = ase->h * ase->frame_count;
  desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
  desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
  desc.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
  desc.data.subimage[0][0].ptr = pixels.data;
  desc.data.subimage[0][0].size = ase->frame_count * rect;
  u32 id = sg_make_image(desc).id;

  Image img = {};
  img.id = id;
  img.width = desc.width;
  img.height = desc.height;

  HashMap<SpriteLoop> by_tag;
  reserve(&by_tag, hash_map_reserve_size((u64)ase->tag_count));

  for (i32 i = 0; i < ase->tag_count; i++) {
    ase_tag_t &tag = ase->tags[i];

    SpriteLoop loop = {};

    for (i32 j = tag.from_frame; j <= tag.to_frame; j++) {
      push(&loop.indices, j);
    }

    u64 key = fnv1a(tag.name, strlen(tag.name));
    by_tag[key] = loop;
  }

  printf("created sprite with image id: %d and %llu frames\n", img.id,
         frames.len);

  Sprite s = {};
  s.img = img;
  s.frames = frames;
  s.by_tag = by_tag;
  s.width = ase->w;
  s.height = ase->h;
  *spr = s;
  return true;
}

void drop(Sprite *spr) {
  drop(&spr->frames);

  for (auto [k, v] : spr->by_tag) {
    drop(&v->indices);
  }
  drop(&spr->by_tag);
}

void sprite_renderer_play(SpriteRenderer *sr, String tag) {
  sr->loop = fnv1a(tag);
  sr->current_frame = 0;
  sr->elapsed = 0;
}

void sprite_renderer_update(SpriteRenderer *sr, float dt) {
  i32 index = 0;
  u64 len = 0;

  Sprite *sprite = &g_app->assets[sr->sprite].sprite;
  SpriteLoop *loop = get(&sprite->by_tag, sr->loop);

  if (loop != nullptr) {
    index = loop->indices[sr->current_frame];
    len = loop->indices.len;
  } else {
    index = sr->current_frame;
    len = sprite->frames.len;
  }

  SpriteFrame frame = sprite->frames[index];

  sr->elapsed += dt * 1000;
  if (sr->elapsed > frame.duration) {
    if (sr->current_frame == len - 1) {
      sr->current_frame = 0;
    } else {
      sr->current_frame++;
    }

    sr->elapsed -= frame.duration;
  }
}

void sprite_renderer_set_frame(SpriteRenderer *sr, i32 frame) {
  i32 len;

  Sprite *sprite = &g_app->assets[sr->sprite].sprite;
  SpriteLoop *loop = get(&sprite->by_tag, sr->loop);

  if (loop != nullptr) {
    len = loop->indices.len;
  } else {
    len = sprite->frames.len;
  }

  if (0 <= frame && frame < len) {
    sr->current_frame = frame;
    sr->elapsed = 0;
  }
}