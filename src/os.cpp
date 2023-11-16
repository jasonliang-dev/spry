#include "os.h"

#if defined(IS_WIN32)
#include <direct.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#elif defined(IS_HTML5)
#include <unistd.h>

#elif defined(IS_LINUX)
#include <sched.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#endif

i32 os_change_dir(const char *path) { return chdir(path); }

String os_program_dir() {
  String str = os_program_path();
  char *buf = str.data;

  for (i32 i = (i32)str.len; i >= 0; i--) {
    if (buf[i] == '/') {
      buf[i + 1] = 0;
      return {str.data, (u64)i + 1};
    }
  }

  return str;
}

#ifdef IS_WIN32

String os_program_path() {
  static char s_buf[2048];

  DWORD len = GetModuleFileNameA(NULL, s_buf, array_size(s_buf));

  for (i32 i = 0; s_buf[i]; i++) {
    if (s_buf[i] == '\\') {
      s_buf[i] = '/';
    }
  }

  return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
  HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                             OPEN_EXISTING, 0, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    return 0;
  }
  defer(CloseHandle(handle));

  FILETIME create = {};
  FILETIME access = {};
  FILETIME write = {};
  bool ok = GetFileTime(handle, &create, &access, &write);
  if (!ok) {
    return 0;
  }

  ULARGE_INTEGER time = {};
  time.LowPart = write.dwLowDateTime;
  time.HighPart = write.dwHighDateTime;

  return time.QuadPart;
}

void os_high_timer_resolution() { timeBeginPeriod(8); }
void os_sleep(u32 ms) { Sleep(ms); }
void os_yield() { YieldProcessor(); }

#endif // IS_WIN32

#ifdef IS_LINUX

String os_program_path() {
  static char s_buf[2048];
  i32 len = (i32)readlink("/proc/self/exe", s_buf, array_size(s_buf));
  return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
  struct stat attrib = {};
  i32 err = stat(filename, &attrib);
  if (err == 0) {
    return (u64)attrib.st_mtime;
  } else {
    return 0;
  }
}

void os_high_timer_resolution() {}

void os_sleep(u32 ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, &ts);
}

void os_yield() { sched_yield(); }

#endif // IS_LINUX
