#pragma once

#include "prelude.h"
#include "slice.h"
#include <atomic>

struct LuaThread {
  Mutex mtx;
  String contents;
  String name;
  Thread thread;

  void make(String code, String thread_name);
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
  void push(lua_State *L);
};

struct LuaTableEntry {
  LuaVariant key;
  LuaVariant value;
};

struct LuaChannel {
  std::atomic<char *> name;

  Mutex mtx;
  Cond received;
  Cond sent;

  u64 received_total;
  u64 sent_total;

  Slice<LuaVariant> items;
  u64 front;
  u64 back;
  u64 len;

  void make(String n, u64 buf);
  void trash();
  void send(LuaVariant item);
  LuaVariant recv();
  bool try_recv(LuaVariant *v);
};

LuaChannel *lua_channel_make(String name, u64 buf);
LuaChannel *lua_channel_get(String name);
LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v);
void lua_channels_shutdown();
