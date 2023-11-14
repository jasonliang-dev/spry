#include "sprite.h"
#include "app.h"
#include "arena.h"
#include "deps/cute_aseprite.h"
#include "deps/sokol_gfx.h"
#include "profile.h"
#include "slice.h"

bool sprite_data_load(SpriteData *spr, Archive *ar, String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  ase_t *ase = nullptr;
  {
    PROFILE_BLOCK("aseprite load");
    ase = cute_aseprite_load_from_memory(contents.data, (i32)contents.len,
                                         nullptr);
  }
  defer(cute_aseprite_free(ase));

  Arena arena = {};

  i32 rect = ase->w * ase->h * 4;

  Slice<SpriteFrame> frames = {};
  slice_from_arena(&frames, &arena, ase->frame_count);

  Array<char> pixels = {};
  array_reserve(&pixels, ase->frame_count * rect);
  defer(array_trash(&pixels));

  for (i32 i = 0; i < ase->frame_count; i++) {
    ase_frame_t &frame = ase->frames[i];

    SpriteFrame sf = {};
    sf.duration = frame.duration_milliseconds;

    sf.u0 = 0;
    sf.v0 = (float)i / ase->frame_count;
    sf.u1 = 1;
    sf.v1 = (float)(i + 1) / ase->frame_count;

    frames[i] = sf;
    memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
  }

  sg_image_desc desc = {};
  desc.width = ase->w;
  desc.height = ase->h * ase->frame_count;
  desc.data.subimage[0][0].ptr = pixels.data;
  desc.data.subimage[0][0].size = ase->frame_count * rect;

  u32 id = 0;
  {
    PROFILE_BLOCK("make image");
    id = sg_make_image(desc).id;
  }

  Image img = {};
  img.id = id;
  img.width = desc.width;
  img.height = desc.height;

  HashMap<SpriteLoop> by_tag = {};
  hashmap_reserve(&by_tag, (u64)ase->tag_count);

  for (i32 i = 0; i < ase->tag_count; i++) {
    ase_tag_t &tag = ase->tags[i];

    u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

    SpriteLoop loop = {};

    slice_from_arena(&loop.indices, &arena, len);
    for (i32 j = 0; j < len; j++) {
      loop.indices[j] = j + tag.from_frame;
    }

    by_tag[fnv1a(tag.name)] = loop;
  }

  printf("created sprite with image id: %d and %llu frames\n", img.id,
         (unsigned long long)frames.len);

  SpriteData s = {};
  s.arena = arena;
  s.img = img;
  s.frames = frames;
  s.by_tag = by_tag;
  s.width = ase->w;
  s.height = ase->h;
  *spr = s;
  return true;
}

void sprite_data_trash(SpriteData *spr) {
  hashmap_trash(&spr->by_tag);
  arena_trash(&spr->arena);
}

bool sprite_play(Sprite *spr, String tag) {
  u64 key = fnv1a(tag);
  bool same = spr->loop == key;
  spr->loop = key;
  return same;
}

void sprite_update(Sprite *spr, float dt) {
  SpriteView view = {};
  bool ok = sprite_view(&view, spr);
  defer(sprite_view_unlock());
  if (!ok) {
    return;
  }

  i32 index = sprite_view_frame(&view);
  SpriteFrame frame = view.data->frames[index];

  spr->elapsed += dt * 1000;
  if (spr->elapsed > frame.duration) {
    if (spr->current_frame == sprite_view_len(&view) - 1) {
      spr->current_frame = 0;
    } else {
      spr->current_frame++;
    }

    spr->elapsed -= frame.duration;
  }
}

void sprite_set_frame(Sprite *spr, i32 frame) {
  SpriteView view = {};
  bool ok = sprite_view(&view, spr);
  defer(sprite_view_unlock());
  if (!ok) {
    return;
  }

  if (0 <= frame && frame < sprite_view_len(&view)) {
    spr->current_frame = frame;
    spr->elapsed = 0;
  }
}

bool sprite_view(SpriteView *out, Sprite *spr) {
  cute_read_lock(&g_app->assets.rw_lock);
  const Asset *a = hashmap_get(&g_app->assets.table, spr->sprite);

  if (a == nullptr) {
    return false;
  }

  const SpriteData *data = &a->sprite;
  if (data == nullptr) {
    return false;
  }

  SpriteLoop *loop = hashmap_get(&data->by_tag, spr->loop);

  SpriteView view = {};
  view.sprite = spr;
  view.data = data;
  view.loop = loop;
  *out = view;
  return true;
}

void sprite_view_unlock() { cute_read_unlock(&g_app->assets.rw_lock); }

i32 sprite_view_frame(SpriteView *view) {
  if (view->loop != nullptr) {
    return view->loop->indices[view->sprite->current_frame];
  } else {
    return view->sprite->current_frame;
  }
}

u64 sprite_view_len(SpriteView *view) {
  if (view->loop != nullptr) {
    return view->loop->indices.len;
  } else {
    return view->data->frames.len;
  }
}
