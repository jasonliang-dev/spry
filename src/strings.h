#pragma once

#include "prelude.h"

String substr(String str, u64 i, i64 count);
bool starts_with(String hay, String match);
bool ends_with(String hay, String match);
u64 first_of(String hay, char c);
u64 last_of(String hay, char c);

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
};

SplitLinesIterator begin(SplitLines sl);
SplitLinesIterator end(SplitLines sl);

i32 utf8_size(u8 c);

struct Rune {
  u32 value;
};

u32 rune_charcode(Rune r);

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
};

UTF8Iterator begin(UTF8 utf8);
UTF8Iterator end(UTF8 utf8);

struct StringBuilder {
  char *data;
  u64 len;      // does not include null term
  u64 capacity; // includes null term
};

StringBuilder string_builder_make();
void drop(StringBuilder *sb);
void reserve(StringBuilder *sb, u64 capacity);
void concat(StringBuilder *sb, String str);
void clear(StringBuilder *sb);
void relative_path(StringBuilder *sb, String filepath, String file);

FORMAT_ARGS(2)
void format(StringBuilder *sb, const char *fmt, ...);

inline String as_string(StringBuilder *sb) { return {sb->data, sb->len}; }
inline void concat(StringBuilder *sb, char *cstr) {
  concat(sb, {cstr, strlen(cstr)});
}
