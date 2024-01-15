#include "image.h"
#include "app.h"
#include "deps/sokol_gfx.h"
#include "deps/stb_image.h"
#include "deps/stb_image_resize2.h"
#include "profile.h"
#include "vfs.h"
#include <stdio.h>

bool Image::load(String filepath, bool generate_mips) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = vfs_read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  i32 width = 0, height = 0, channels = 0;
  stbi_uc *data = nullptr;
  {
    PROFILE_BLOCK("stb_image load");
    data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &width,
                                 &height, &channels, 4);
  }
  if (!data) {
    return false;
  }
  defer(stbi_image_free(data));

  sg_image_desc desc = {};
  desc.pixel_format = SG_PIXELFORMAT_RGBA8;
  desc.width = width;
  desc.height = height;
  desc.data.subimage[0][0].ptr = data;
  desc.data.subimage[0][0].size = width * height * 4;

  Array<u8 *> mips = {};
  defer({
    for (u8 *mip : mips) {
      mem_free(mip);
    }
    mips.trash();
  });

  if (generate_mips) {
    mips.reserve(SG_MAX_MIPMAPS);

    u8 *prev = data;
    i32 w0 = width;
    i32 h0 = height;
    i32 w1 = w0 / 2;
    i32 h1 = h0 / 2;

    while (w1 > 1 && h1 > 1) {
      PROFILE_BLOCK("generate mip");

      u8 *mip = (u8 *)mem_alloc(w1 * h1 * 4);
      stbir_resize_uint8_linear(prev, w0, h0, 0, mip, w1, h1, 0, STBIR_RGBA);
      mips.push(mip);

      desc.data.subimage[0][mips.len].ptr = mip;
      desc.data.subimage[0][mips.len].size = w1 * h1 * 4;

      prev = mip;
      w0 = w1;
      h0 = h1;
      w1 /= 2;
      h1 /= 2;
    }
  }

  desc.num_mipmaps = mips.len + 1;

  u32 id = 0;
  {
    PROFILE_BLOCK("make image");
    LockGuard lock{&g_app->gpu_mtx};
    id = sg_make_image(desc).id;
  }

  Image img = {};
  img.id = id;
  img.width = width;
  img.height = height;
  img.has_mips = generate_mips;
  *this = img;

  printf("created image (%dx%d, %d channels, mipmaps: %s) with id %d\n", width,
         height, channels, generate_mips ? "true" : "false", id);
  return true;
}

void Image::trash() {
  LockGuard lock{&g_app->gpu_mtx};
  sg_destroy_image({id});
}
