#include "vfs.h"
#include "app.h"
#include "deps/miniz.h"
#include "deps/tinydir.h"
#include "os.h"
#include "prelude.h"
#include "profile.h"
#include "strings.h"
#include <new>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct FileSystem {
  virtual ~FileSystem() = 0;
  virtual bool file_exists(String filepath) = 0;
  virtual bool read_entire_file(String *out, String filepath) = 0;
  virtual bool list_all_files(Array<String> *files) = 0;
};
FileSystem::~FileSystem() {}

static FileSystem *g_filesystem;

static bool read_entire_file_raw(String *out, String filepath) {
  PROFILE_FUNC();

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

static bool list_all_files_help(Array<String> *files, String path) {
  PROFILE_FUNC();

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
        String s = str_fmt("%s%s/", path.data, file.name);
        defer(mem_free(s.data));
        list_all_files_help(files, s);
      } else {
        array_push(files, str_fmt("%s%s", path.data, file.name));
      }
    }

    tinydir_next(&dir);
  }

  return true;
}

struct DirectoryFileSystem : FileSystem {
  ~DirectoryFileSystem() {}

  bool file_exists(String filepath) {
    PROFILE_FUNC();

    String path = to_cstr(filepath);
    defer(mem_free(path.data));

    FILE *fp = fopen(path.data, "r");
    if (fp != nullptr) {
      fclose(fp);
      return true;
    }

    return false;
  }

  bool read_entire_file(String *out, String filepath) {
    return read_entire_file_raw(out, filepath);
  }

  bool list_all_files(Array<String> *files) {
    return list_all_files_help(files, "");
  }
};

struct ZipFileSystem : FileSystem {
  mz_zip_archive zip = {};
  String zip_contents = {};

  ~ZipFileSystem() {
    if (zip_contents.data != nullptr) {
      mz_zip_reader_end(&zip);
      mem_free(zip_contents.data);
    }
  }

  bool file_exists(String filepath) {
    PROFILE_FUNC();

    String path = to_cstr(filepath);
    defer(mem_free(path.data));

    i32 i = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
    if (i == -1) {
      return false;
    }

    mz_zip_archive_file_stat stat;
    mz_bool ok = mz_zip_reader_file_stat(&zip, i, &stat);
    if (!ok) {
      return false;
    }

    return true;
  }

  bool read_entire_file(String *out, String filepath) {
    PROFILE_FUNC();

    String path = to_cstr(filepath);
    defer(mem_free(path.data));

    i32 file_index = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
    if (file_index == -1) {
      return false;
    }

    mz_zip_archive_file_stat stat;
    mz_bool ok = mz_zip_reader_file_stat(&zip, file_index, &stat);
    if (!ok) {
      return false;
    }

    size_t size = stat.m_uncomp_size;
    char *buf = (char *)mem_alloc(size + 1);

    ok = mz_zip_reader_extract_to_mem(&zip, file_index, buf, size, 0);
    if (!ok) {
      mz_zip_error err = mz_zip_get_last_error(&zip);
      fprintf(stderr, "failed to read file '%s': %s\n", path.data,
              mz_zip_get_error_string(err));
      mem_free(buf);
      return false;
    }

    buf[size] = 0;
    *out = {buf, size};
    return true;
  }

  bool list_all_files(Array<String> *files) {
    PROFILE_FUNC();

    for (u32 i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
      mz_zip_archive_file_stat file_stat;
      mz_bool ok = mz_zip_reader_file_stat(&zip, i, &file_stat);
      if (!ok) {
        return false;
      }

      String name = {file_stat.m_filename, strlen(file_stat.m_filename)};
      array_push(files, to_cstr(name));
    }

    return true;
  }
};

static bool vfs_mount_directory(String mount) {
  PROFILE_FUNC();

  String path = to_cstr(mount);
  defer(mem_free(path.data));

  if (os_change_dir(path.data) != 0) {
    return false;
  }

  g_filesystem = (DirectoryFileSystem *)mem_alloc(sizeof(DirectoryFileSystem));
  new (g_filesystem) DirectoryFileSystem();
  return true;
}

static u32 read4(char *bytes) {
  u32 n;
  memcpy(&n, bytes, 4);
  return n;
}

static bool vfs_mount_zip(String mount) {
  PROFILE_FUNC();

  String contents = {};
  bool contents_ok = read_entire_file_raw(&contents, mount);
  if (!contents_ok) {
    return false;
  }

  ZipFileSystem *vfs = (ZipFileSystem *)mem_alloc(sizeof(ZipFileSystem));
  new (vfs) ZipFileSystem();

  bool success = false;
  defer({
    if (!success) {
      mem_free(contents.data);

      if (vfs != nullptr) {
        vfs->~ZipFileSystem();
        mem_free(vfs);
      }
    }
  });

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

  mz_bool zip_ok = mz_zip_reader_init_mem(&vfs->zip, begin, zip_len, 0);
  if (!zip_ok) {
    mz_zip_error err = mz_zip_get_last_error(&vfs->zip);
    fprintf(stderr, "failed to read zip: %s\n", mz_zip_get_error_string(err));
    return false;
  }

  vfs->zip_contents = contents;

  g_filesystem = vfs;
  success = true;
  return true;
}

#ifdef __EMSCRIPTEN__
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(spryMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
  var dirs = spryMount.split('/');
  dirs.pop();

  var path = [];
  for (var dir of dirs) {
    path.push(dir);
    FS.mkdir(path.join('/'));
  }

  await fetch(spryMount).then(async function(res) {
    if (!res.ok) {
      throw new Error('failed to fetch ' + spryMount);
    }

    var data = await res.arrayBuffer();
    FS.writeFile(spryMount, new Uint8Array(data));
  });
});

EM_ASYNC_JS(void, web_load_files, (), {
  var jobs = [];

  function spryWalkFiles(files, leading) {
    var path = leading.join('/');
    if (path != '') {
      FS.mkdir(path);
    }

    for (var entry of Object.entries(files)) {
      var key = entry[0];
      var value = entry[1];
      var filepath = [... leading, key ];
      if (typeof value == 'object') {
        spryWalkFiles(value, filepath);
      } else if (value == 1) {
        var file = filepath.join('/');

        var job = fetch(file).then(async function(res) {
          if (!res.ok) {
            throw new Error('failed to fetch ' + file);
          }
          var data = await res.arrayBuffer();
          FS.writeFile(file, new Uint8Array(data));
        });

        jobs.push(job);
      }
    }
  }
  spryWalkFiles(spryFiles, []);

  await Promise.all(jobs);
});
#endif

MountResult vfs_mount(int argc, char **argv) {
  PROFILE_FUNC();

  MountResult res = {};

#ifdef __EMSCRIPTEN__
  String mount_dir = web_mount_dir();
  defer(free(mount_dir.data));

  if (ends_with(mount_dir, ".zip")) {
    web_load_zip();
    res.ok = vfs_mount_zip(mount_dir);
  } else {
    web_load_files();
    res.ok = vfs_mount_directory(mount_dir);
  }

#else
  if (argc == 1) {
    String path = os_program_path();

#ifdef DEBUG
    printf("program path: %s\n", path.data);
#endif

    res.ok = vfs_mount_zip(path);
  } else if (argc == 2) {
    String mount_dir = argv[1];

    if (ends_with(mount_dir, ".zip")) {
      res.ok = vfs_mount_zip(mount_dir);
    } else {
      res.ok = vfs_mount_directory(mount_dir);
      res.can_hot_reload = true;
    }
  }
#endif

  if (argc == 2 && !res.ok) {
    fatal_error(tmp_fmt("failed to load: %s", argv[1]));
  }

  return res;
}

void vfs_trash() {
  if (g_filesystem != nullptr) {
    g_filesystem->~FileSystem();
    mem_free(g_filesystem);
  }
}

bool vfs_file_exists(String filepath) {
  return g_filesystem->file_exists(filepath);
}

bool vfs_read_entire_file(String *out, String filepath) {
  return g_filesystem->read_entire_file(out, filepath);
}

bool vfs_list_all_files(Array<String> *files) {
  return g_filesystem->list_all_files(files);
}
