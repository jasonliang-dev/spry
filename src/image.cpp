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

  i32 width, height, channels;
  stbi_uc *data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len,
                                        &width, &height, &channels, 0);
  if (!data) {
    return false;
  }
  defer(stbi_image_free(data));
  assert(channels == 4);

  sg_image_desc sg_image = {};
  sg_image.width = width;
  sg_image.height = height;
  sg_image.data.subimage[0][0].ptr = data;
  sg_image.data.subimage[0][0].size = width * height * 4;
  u32 id = sg_make_image(sg_image).id;

  printf("created image with id %d\n", id);

  Image img = {};
  img.id = id;
  img.width = width;
  img.height = height;
  *image = img;
  return true;
}

void drop(Image *image) { sg_destroy_image({image->id}); }