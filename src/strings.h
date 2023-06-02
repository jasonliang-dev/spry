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
