#include "atlas.h"
#include "scanner.h"
#include "strings.h"

bool atlas_load(Atlas *atlas, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
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
      defer(string_builder_trash(&sb));

      Scanner scan = make_scanner(line);
      scan_next_string(&scan); // discard 'a'
      String filename = scan_next_string(&scan);

      string_builder_swap_filename(&sb, filepath, filename);

      bool ok = image_load(&img, ar, string_builder_as_string(&sb));
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
      scan_next_string(&scan); // discard 's'
      String name = scan_next_string(&scan);
      scan_next_string(&scan); // discard origin x
      scan_next_string(&scan); // discard origin y
      i32 x = scan_next_int(&scan);
      i32 y = scan_next_int(&scan);
      i32 width = scan_next_int(&scan);
      i32 height = scan_next_int(&scan);
      i32 padding = scan_next_int(&scan);
      i32 trimmed = scan_next_int(&scan);
      scan_next_int(&scan); // discard trim x
      scan_next_int(&scan); // discard trim y
      i32 trim_width = scan_next_int(&scan);
      i32 trim_height = scan_next_int(&scan);

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
         (unsigned long long)by_name.load);

  Atlas a;
  a.by_name = by_name;
  a.img = img;
  *atlas = a;

  return true;
}

void atlas_trash(Atlas *atlas) {
  hashmap_trash(&atlas->by_name);
  image_trash(&atlas->img);
}

AtlasImage *atlas_get(Atlas *atlas, String name) {
  u64 key = fnv1a(name);
  return hashmap_get(&atlas->by_name, key);
}
