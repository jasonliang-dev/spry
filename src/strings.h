#pragma once

#include "prelude.h"

struct SplitLinesIterator {
  String data;
  String view;

  String operator*() const { return view; }
  SplitLinesIterator &operator++();
};

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs);

struct SplitLines {
  String str;
  SplitLines(String s) : str(s) {}

  SplitLinesIterator begin();
  SplitLinesIterator end();
};

i32 utf8_size(u8 c);

struct Rune {
  u32 value;

  u32 charcode();
  bool is_whitespace();
  bool is_digit();
};

Rune rune_from_string(const char *buf);

struct UTF8Iterator {
  String str;
  u64 cursor;
  Rune rune;

  Rune operator*() const { return rune; }
  UTF8Iterator &operator++();
};

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs);

struct UTF8 {
  String str;
  UTF8(String s) : str(s) {}

  UTF8Iterator begin();
  UTF8Iterator end();
};

struct StringBuilder {
  char *data;
  u64 len;      // does not include null term
  u64 capacity; // includes null term

  StringBuilder();

  void trash();
  void reserve(u64 capacity);
  void clear();
  void swap_filename(String filepath, String file);
  void concat(String str, i32 times);

  StringBuilder &operator<<(String str);
  explicit operator String();
};

FORMAT_ARGS(1) String str_fmt(const char *fmt, ...);
FORMAT_ARGS(1) String tmp_fmt(const char *fmt, ...);

double string_to_double(String str);
