#include "image.h"
#include "archive.h"
#include "deps/sokol_gfx.h"
#include "deps/stb_image.h"
#include <stdio.h>

bool image_load(Image *image, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  i32 width = 0, height = 0, channels = 0;
  stbi_uc *data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len,
                                        &width, &height, &channels, 4);
  if (!data) {
    return false;
  }
  defer(stbi_image_free(data));

  u8 *img_data = data;
  if (channels == 3) {
    u8 *data4 = (u8 *)mem_alloc(width * height * 4);
    for (i32 i = 0; i < width * height; i++) {
      data4[i * 4 + 0] = data[i * 3 + 0];
      data4[i * 4 + 1] = data[i * 3 + 1];
      data4[i * 4 + 2] = data[i * 3 + 2];
      data4[i * 4 + 3] = 255;
    }
    img_data = data4;
  }
  defer({
    if (channels == 3) {
      mem_free(img_data);
    }
  });

  sg_image_desc desc = {};
  desc.pixel_format = SG_PIXELFORMAT_RGBA8;
  desc.width = width;
  desc.height = height;
  desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
  desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
  desc.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
  desc.data.subimage[0][0].ptr = data;
  desc.data.subimage[0][0].size = width * height * 4;
  u32 id = sg_make_image(desc).id;

  printf("created image (%dx%d) with id %d\n", width, height, id);

  Image img = {};
  img.id = id;
  img.width = width;
  img.height = height;
  *image = img;
  return true;
}

void drop(Image *image) { sg_destroy_image({image->id}); }