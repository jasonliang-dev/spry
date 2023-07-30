#include "archive.h"
#include "deps/tinydir.h"
#include "prelude.h"
#include "strings.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
#include <unistd.h>
#endif

static bool read_entire_file_raw(String *out, String filepath) {
  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  FILE *file = fopen(path.data, "rb");
  if (file == nullptr) {
    return false;
  }

  fseek(file, 0L, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char *buf = (char *)mem_alloc(size + 1);
  size_t read = fread(buf, sizeof(char), size, file);
  fclose(file);

  if (read != size) {
    mem_free(buf);
    return false;
  }

  buf[size] = 0;
  *out = {buf, size};
  return true;
}

static bool list_all_files(Array<String> *files, String path) {
  tinydir_dir dir;
  if (path.len == 0) {
    tinydir_open(&dir, ".");
  } else {
    tinydir_open(&dir, path.data);
  }
  defer(tinydir_close(&dir));

  while (dir.has_next) {
    tinydir_file file;
    tinydir_readfile(&dir, &file);

    if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0) {
      if (file.is_dir) {
        StringBuilder sb = format("%s%s/", path.data, file.name);
        defer(drop(&sb));

        list_all_files(files, as_string(&sb));
      } else {
        StringBuilder sb = format("%s%s", path.data, file.name);
        push(files, as_string(&sb));
      }
    }

    tinydir_next(&dir);
  }

  return true;
}

static bool fs_archive_file_exists(Archive *self, String filepath) {
  (void)self;
  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  FILE *fp = fopen(path.data, "r");
  if (fp != nullptr) {
    fclose(fp);
    return true;
  }

  return false;
};

static bool fs_archive_read_entire_file(Archive *self, String *out,
                                        String filepath) {
  (void)self;
  return read_entire_file_raw(out, filepath);
}

static bool fs_archive_list_all_files(Archive *self, Array<String> *files) {
  (void)self;
  return list_all_files(files, "");
}

bool load_filesystem_archive(Archive *ar, String mount) {
  String path = to_cstr(mount);
  defer(mem_free(path.data));

  if (chdir(path.data) != 0) {
    return false;
  }

  *ar = {};
  ar->file_exists = fs_archive_file_exists;
  ar->read_entire_file = fs_archive_read_entire_file;
  ar->list_all_files = fs_archive_list_all_files;

  return true;
}

static u32 read4(char *bytes) {
  u32 n;
  memcpy(&n, bytes, 4);
  return n;
}

static bool zip_archive_file_exists(Archive *self, String filepath) {
  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  i32 i = mz_zip_reader_locate_file(&self->zip, path.data, nullptr, 0);
  if (i == -1) {
    return false;
  }

  mz_zip_archive_file_stat stat;
  mz_bool ok = mz_zip_reader_file_stat(&self->zip, i, &stat);
  if (!ok) {
    return false;
  }

  return true;
};

static bool zip_archive_read_entire_file(Archive *self, String *out,
                                         String filepath) {
  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  i32 file_index = mz_zip_reader_locate_file(&self->zip, path.data, nullptr, 0);
  if (file_index == -1) {
    return false;
  }

  mz_zip_archive_file_stat stat;
  mz_bool ok = mz_zip_reader_file_stat(&self->zip, file_index, &stat);
  if (!ok) {
    return false;
  }

  usize size = stat.m_uncomp_size;
  char *buf = (char *)mem_alloc(size + 1);

  ok = mz_zip_reader_extract_to_mem(&self->zip, file_index, buf, size, 0);
  if (!ok) {
    mz_zip_error err = mz_zip_get_last_error(&self->zip);
    fprintf(stderr, "failed to read file '%s': %s\n", path.data,
            mz_zip_get_error_string(err));
    mem_free(buf);
    return false;
  }

  buf[size] = 0;
  *out = {buf, size};
  return true;
}

static bool zip_archive_list_all_files(Archive *self, Array<String> *files) {
  for (u32 i = 0; i < mz_zip_reader_get_num_files(&self->zip); i++) {
    mz_zip_archive_file_stat file_stat;
    mz_bool ok = mz_zip_reader_file_stat(&self->zip, i, &file_stat);
    if (!ok) {
      return false;
    }

    String name = {file_stat.m_filename, strlen(file_stat.m_filename)};
    push(files, to_cstr(name));
  }

  return true;
}

bool load_zip_archive(Archive *ar, String mount) {
  String contents;
  bool contents_ok = read_entire_file_raw(&contents, mount);
  if (!contents_ok) {
    return false;
  }

  char *data = contents.data;
  char *end = &data[contents.len];

  constexpr i32 eocd_size = 22;
  char *eocd = end - eocd_size;
  if (read4(eocd) != 0x06054b50) {
    fprintf(stderr, "can't find EOCD record\n");
    return false;
  }

  u32 central_size = read4(&eocd[12]);
  if (read4(eocd - central_size) != 0x02014b50) {
    fprintf(stderr, "can't find central directory\n");
    return false;
  }

  u32 central_offset = read4(&eocd[16]);
  char *begin = eocd - central_size - central_offset;
  u64 zip_len = end - begin;
  if (read4(begin) != 0x04034b50) {
    fprintf(stderr, "can't read local file header\n");
    return false;
  }

  *ar = {};

  mz_bool zip_ok = mz_zip_reader_init_mem(&ar->zip, begin, zip_len, 0);
  if (!zip_ok) {
    mz_zip_error err = mz_zip_get_last_error(&ar->zip);
    fprintf(stderr, "failed to read zip: %s\n", mz_zip_get_error_string(err));
    return false;
  }

  ar->file_exists = zip_archive_file_exists;
  ar->read_entire_file = zip_archive_read_entire_file;
  ar->list_all_files = zip_archive_list_all_files;
  ar->zip_contents = contents;

  return true;
}

void drop(Archive *ar) {
  if (ar->zip_contents.data != nullptr) {
    mz_zip_reader_end(&ar->zip);
    mem_free(ar->zip_contents.data);
  }
}

String program_dir() {
  String str = program_path();
  char *buf = str.data;

  for (i32 i = (i32)str.len; i >= 0; i--) {
    if (buf[i] == '/') {
      buf[i + 1] = 0;
      return {str.data, (u64)i + 1};
    }
  }

  return str;
}

String program_path() {
  static char s_buf[2048];

#ifdef _WIN32
  DWORD len = GetModuleFileNameA(NULL, s_buf, array_size(s_buf));

  for (i32 i = 0; s_buf[i]; i++) {
    if (s_buf[i] == '\\') {
      s_buf[i] = '/';
    }
  }
#endif

#ifdef __linux__
  i32 len = (i32)readlink("/proc/self/exe", s_buf, array_size(s_buf));
#endif

#ifdef __EMSCRIPTEN__
  i32 len = 0;
#endif

  return {s_buf, (u64)len};
}