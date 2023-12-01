#pragma once

#include "prelude.h"

String substr(String str, u64 i, u64 j);
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

StringBuilder string_builder_make();
void string_builder_trash(StringBuilder *sb);
void string_builder_reserve(StringBuilder *sb, u64 capacity);
void string_builder_clear(StringBuilder *sb);
void string_builder_swap_filename(StringBuilder *sb, String filepath,
                                  String file);
void string_builder_concat(StringBuilder *sb, String str, i32 times);

StringBuilder &operator<<(StringBuilder &sb, String str);

#define BUILD_STRING(sbuilder)                                                 \
  StringBuilder sbuilder = string_builder_make();                              \
  defer(string_builder_trash(&sbuilder));

FORMAT_ARGS(1) String str_fmt(const char *fmt, ...);
FORMAT_ARGS(1) String tmp_fmt(const char *fmt, ...);

double string_to_double(String str);
