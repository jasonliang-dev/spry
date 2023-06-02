#include "atlas.h"
#include "scanner.h"
#include "strings.h"

bool atlas_load(Atlas *atlas, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  Image img = {};
  HashMap<AtlasImage> by_name = {};

  for (String line : SplitLines(contents)) {
    switch (line.data[0]) {
    case 'a': {
      StringBuilder sb = string_builder_make();
      defer(drop(&sb));

      Scanner scan = make_scanner(line);
      next_string(&scan); // discard 'a'
      String filename = next_string(&scan);

      relative_path(&sb, filepath, filename);

      bool ok = image_load(&img, ar, as_string(&sb));
      if (!ok) {
        return false;
      }
      break;
    }
    case 's': {
      if (img.id == 0) {
        return false;
      }

      Scanner scan = make_scanner(line);
      next_string(&scan); // discard 's'
      String name = next_string(&scan);
      next_string(&scan); // discard origin x
      next_string(&scan); // discard origin y
      i32 x = next_int(&scan);
      i32 y = next_int(&scan);
      i32 width = next_int(&scan);
      i32 height = next_int(&scan);
      i32 padding = next_int(&scan);
      i32 trimmed = next_int(&scan);
      next_int(&scan); // discard trim x
      next_int(&scan); // discard trim y
      i32 trim_width = next_int(&scan);
      i32 trim_height = next_int(&scan);

      AtlasImage atlas_img = {};
      atlas_img.img = img;
      atlas_img.u0 = (x + padding) / (float)img.width;
      atlas_img.v0 = (y + padding) / (float)img.height;

      if (trimmed != 0) {
        atlas_img.width = (float)trim_width;
        atlas_img.height = (float)trim_height;
        atlas_img.u1 = (x + padding + trim_width) / (float)img.width;
        atlas_img.v1 = (y + padding + trim_height) / (float)img.height;
      } else {
        atlas_img.width = (float)width;
        atlas_img.height = (float)height;
        atlas_img.u1 = (x + padding + width) / (float)img.width;
        atlas_img.v1 = (y + padding + height) / (float)img.height;
      }

      by_name[fnv1a(name)] = atlas_img;

      break;
    }
    default: break;
    }
  }

  printf("created atlas with image id: %d and %llu entries\n", img.id,
         by_name.load);

  Atlas a;
  a.by_name = by_name;
  a.img = img;
  *atlas = a;

  return true;
}

void drop(Atlas *atlas) {
  drop(&atlas->by_name);
  drop(&atlas->img);
}

AtlasImage *atlas_get(Atlas *atlas, String name) {
  u64 key = fnv1a(name);
  return get(&atlas->by_name, key);
}
