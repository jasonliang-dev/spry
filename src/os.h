#pragma once

#include "prelude.h"

i32 os_change_dir(const char *path);
String os_program_dir();
String os_program_path();
u64 os_file_modtime(const char *filename);
void os_high_timer_resolution();
void os_sleep(u32 ms);
void os_yield();
