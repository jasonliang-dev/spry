#pragma once

#include "array.h"
#include "prelude.h"

struct MountResult {
  bool ok;
  bool can_hot_reload;
  bool is_fused;
};

MountResult vfs_mount(const char *filepath);
void vfs_trash();

bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String *out, String filepath);
bool vfs_write_entire_file(String filepath, String contents);
bool vfs_list_all_files(Array<String> *files);

void *vfs_for_miniaudio();