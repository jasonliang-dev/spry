#pragma once

#include "prelude.h"

struct LuaThread {
  String contents;
  String error;
  void *thread;

  void join();
};

LuaThread *lua_thread_make(String contents);