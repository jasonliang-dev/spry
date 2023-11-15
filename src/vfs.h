#pragma once

#include "array.h"
#include "prelude.h"

struct MountResult {
  bool ok;
  bool can_hot_reload;
};

MountResult vfs_mount(int argc, char **argv);
void vfs_trash();

bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String *out, String filepath);
bool vfs_list_all_files(Array<String> *files);
