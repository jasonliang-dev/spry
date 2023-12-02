#include "atlas.h"
#include "profile.h"
#include "scanner.h"
#include "strings.h"
#include "vfs.h"

bool Atlas::load(String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = vfs_read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  Image img = {};
  HashMap<AtlasImage> by_name = {};

  for (String line : SplitLines(contents)) {
    switch (line.data[0]) {
    case 'a': {
      Scanner scan = line;
      scan.next_string(); // discard 'a'
      String filename = scan.next_string();

      StringBuilder sb = {};
      defer(sb.trash());
      sb.swap_filename(filepath, filename);
      bool ok = img.load(sb);
      if (!ok) {
        return false;
      }
      break;
    }
    case 's': {
      if (img.id == 0) {
        return false;
      }

      Scanner scan = line;
      scan.next_string(); // discard 's'
      String name = scan.next_string();
      scan.next_string(); // discard origin x
      scan.next_string(); // discard origin y
      i32 x = scan.next_int();
      i32 y = scan.next_int();
      i32 width = scan.next_int();
      i32 height = scan.next_int();
      i32 padding = scan.next_int();
      i32 trimmed = scan.next_int();
      scan.next_int(); // discard trim x
      scan.next_int(); // discard trim y
      i32 trim_width = scan.next_int();
      i32 trim_height = scan.next_int();

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
  *this = a;

  return true;
}

void Atlas::trash() {
  by_name.trash();
  img.trash();
}

AtlasImage *Atlas::get(String name) {
  u64 key = fnv1a(name);
  return by_name.get(key);
}
