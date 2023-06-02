#pragma once

#include "array.h"
#include "deps/miniz.h"
#include "prelude.h"

struct Archive {
  bool (*file_exists)(Archive *self, String filepath);
  bool (*read_entire_file)(Archive *self, String *out, String filepath);
  bool (*list_all_files)(Archive *self, Array<String> *files);

  mz_zip_archive zip;
  String zip_contents;
};

bool make_filesystem_archive(Archive *ar, String mount);
bool make_zip_archive(Archive *ar, String mount);
void drop(Archive *ar);
String program_dir();
String program_path();