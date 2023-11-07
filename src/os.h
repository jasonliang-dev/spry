#pragma once

#include "prelude.h"

#if defined(_WIN32)
#define IS_WIN32

#elif defined(__EMSCRIPTEN__)
#define IS_HTML5

#elif defined(__linux__) || defined(__unix__)
#define IS_LINUX

#endif

i32 os_change_dir(const char *path);
String os_program_dir();
String os_program_path();
u64 os_file_modtime(const char *filename);
void os_high_timer_resolution();
void os_sleep(u32 ms);
void os_yield();
i32 os_process_id();
i32 os_thread_id();
