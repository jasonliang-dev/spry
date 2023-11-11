#pragma once

#include "array.h"
#include "prelude.h"

struct Archive {
  virtual void trash() = 0;
  virtual bool file_exists(String filepath) = 0;
  virtual bool read_entire_file(String *out, String filepath) = 0;
  virtual bool list_all_files(Array<String> *files) = 0;
};

Archive *load_filesystem_archive(String mount);
Archive *load_zip_archive(String mount);
