#include "sprite.h"
#include "app.h"
#include "arena.h"
#include "assets.h"
#include "deps/cute_aseprite.h"
#include "deps/sokol_gfx.h"
#include "profile.h"
#include "slice.h"
#include "vfs.h"

bool SpriteData::load(String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = vfs_read_entire_file(&contents, filepath);
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
  frames.resize(&arena, ase->frame_count);

  Array<char> pixels = {};
  pixels.reserve(ase->frame_count * rect);
  defer(pixels.trash());

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
    LockGuard lock{&g_app->gpu_mtx};
    id = sg_make_image(desc).id;
  }

  Image img = {};
  img.id = id;
  img.width = desc.width;
  img.height = desc.height;

  HashMap<SpriteLoop> by_tag = {};
  by_tag.reserve(ase->tag_count);

  for (i32 i = 0; i < ase->tag_count; i++) {
    ase_tag_t &tag = ase->tags[i];

    u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

    SpriteLoop loop = {};

    loop.indices.resize(&arena, len);
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
  *this = s;
  return true;
}

void SpriteData::trash() {
  by_tag.trash();
  arena.trash();
}

bool Sprite::play(String tag) {
  u64 key = fnv1a(tag);
  bool same = loop == key;
  loop = key;
  return same;
}

void Sprite::update(float dt) {
  SpriteView view = {};
  bool ok = view.make(this);
  if (!ok) {
    return;
  }

  i32 index = view.frame();
  SpriteFrame frame = view.data.frames[index];

  elapsed += dt * 1000;
  if (elapsed > frame.duration) {
    if (current_frame == view.len() - 1) {
      current_frame = 0;
    } else {
      current_frame++;
    }

    elapsed -= frame.duration;
  }
}

void Sprite::set_frame(i32 frame) {
  SpriteView view = {};
  bool ok = view.make(this);
  if (!ok) {
    return;
  }

  if (0 <= frame && frame < view.len()) {
    current_frame = frame;
    elapsed = 0;
  }
}

bool SpriteView::make(Sprite *spr) {
  Asset a = {};
  bool ok = asset_read(spr->sprite, &a);
  if (!ok) {
    return false;
  }

  SpriteData data = a.sprite;
  const SpriteLoop *res = data.by_tag.get(spr->loop);

  SpriteView view = {};
  view.sprite = spr;
  view.data = data;

  if (res != nullptr) {
    view.loop = *res;
  }

  *this = view;
  return true;
}

i32 SpriteView::frame() {
  if (loop.indices.data != nullptr) {
    return loop.indices[sprite->current_frame];
  } else {
    return sprite->current_frame;
  }
}

u64 SpriteView::len() {
  if (loop.indices.data != nullptr) {
    return loop.indices.len;
  } else {
    return data.frames.len;
  }
}
