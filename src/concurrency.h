#pragma once

#include "prelude.h"
#include "slice.h"

struct LuaThread {
  String contents;
  String error;
  void *thread;

  void join();
};

LuaThread *lua_thread_make(String contents);

struct LuaThreadValue {
  int type;
  union {
    double number;
    String string;
  };
};

struct lua_State;
LuaThreadValue lua_thread_value(lua_State *L, i32 arg);

struct LuaChannel {
  Mutex mtx;
  Cond received;
  Cond sent;

  u64 received_total;
  u64 sent_total;

  Slice<LuaThreadValue> items;
  u64 front;
  u64 back;
  u64 len;

  void trash();
  void send(LuaThreadValue item);
  LuaThreadValue recv();
};

LuaChannel *lua_channel_make(String name, u64 cap);
LuaChannel *lua_channel_get(String name);
void lua_channels_shutdown();

