#pragma once

#include "prelude.h"
#include "slice.h"
#include <atomic>

struct LuaThread {
  String contents;
  String name;
  String error;
  std::atomic<Thread> thread;

  void make(String code, String thread_name);
  void trash();
  void join();
};

struct lua_State;
struct LuaTableEntry;
struct LuaVariant {
  i32 type;
  union {
    bool boolean;
    double number;
    String string;
    Slice<LuaTableEntry> table;
  };

  void make(lua_State *L, i32 arg);
  void trash();
  int push(lua_State *L);
};

struct LuaTableEntry {
  LuaVariant key;
  LuaVariant value;
};

struct LuaChannel {
  Mutex mtx;
  Cond received;
  Cond sent;

  u64 received_total;
  u64 sent_total;

  Slice<LuaVariant> items;
  u64 front;
  u64 back;
  u64 len;

  void trash();
  void send(LuaVariant item);
  LuaVariant recv();
};

LuaChannel *lua_channel_make(String name, u64 cap);
LuaChannel *lua_channel_get(String name);
void lua_channels_shutdown();
