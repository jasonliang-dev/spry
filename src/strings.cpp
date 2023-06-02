#include "strings.h"
#include <stdarg.h>
#include <stdio.h>

String substr(String str, u64 i, i64 count) {
  if (count <= 0) {
    i64 len = (i64)str.len - (i64)i + count;
    assert(len >= 0);
    return {&str.data[i], (u64)len};
  } else {
    assert((i64)i + count <= (i64)str.len);
    return {&str.data[i], (u64)count};
  }
}

bool starts_with(String hay, String match) {
  if (hay.len < match.len) {
    return false;
  }
  return substr(hay, 0, match.len) == match;
}

bool ends_with(String hay, String match) {
  if (hay.len < match.len) {
    return false;
  }
  return substr(hay, hay.len - match.len, match.len) == match;
}

u64 first_of(String hay, char c) {
  for (u64 i = 0; i < hay.len; i++) {
    if (hay.data[i] == c) {
      return i;
    }
  }

  return (u64)-1;
}

u64 last_of(String hay, char c) {
  if (hay.len == 0) {
    return (u64)-1;
  }

  for (u64 i = hay.len - 1; i > 1; i--) {
    if (hay.data[i] == c) {
      return i;
    }
  }

  if (hay.data[0] == c) {
    return 0;
  }

  return (u64)-1;
}

SplitLinesIterator &SplitLinesIterator::operator++() {
  if (&view.data[view.len] == &data.data[data.len]) {
    view = {&data.data[data.len], 0};
    return *this;
  }

  String next = {};
  next.data = view.data + view.len + 1;

  u64 end = 0;
  while (&next.data[end] < &data.data[data.len] && next.data[end] != '\n' &&
         next.data[end] != 0) {
    end++;
  }
  next.len = end;

  view = next;
  return *this;
}

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs) {
  return lhs.data.data != rhs.data.data || lhs.data.len != rhs.data.len ||
         lhs.view.data != rhs.view.data || lhs.view.len != rhs.view.len;
}

SplitLinesIterator begin(SplitLines sl) {
  char *data = sl.str.data;
  u64 end = 0;
  while (data[end] != '\n' && data[end] != 0) {
    end++;
  }

  String view = {sl.str.data, end};
  return {sl.str, view};
}

SplitLinesIterator end(SplitLines sl) {
  String str = sl.str;
  String view = {str.data + str.len, 0};
  return {str, view};
}

static char s_empty[1] = {0};

StringBuilder string_builder_make() {
  StringBuilder sb = {};
  sb.data = s_empty;
  return sb;
}

void drop(StringBuilder *sb) {
  if (sb->data != s_empty) {
    mem_free(sb->data);
  }
}

void reserve(StringBuilder *sb, u64 capacity) {
  if (capacity > sb->capacity) {
    char *buf = (char *)mem_alloc(capacity);
    memset(buf, 0, capacity);
    memcpy(buf, sb->data, sb->len);

    if (sb->data != s_empty) {
      mem_free(sb->data);
    }

    sb->data = buf;
    sb->capacity = capacity;
  }
}

void concat(StringBuilder *sb, String str) {
  u64 desired = sb->len + str.len + 1;
  u64 capacity = sb->capacity;

  if (desired >= capacity) {
    u64 growth = capacity > 0 ? capacity * 2 : 8;
    if (growth <= desired) {
      growth = desired;
    }

    reserve(sb, growth);
  }

  memcpy(&sb->data[sb->len], str.data, str.len);
  sb->len += str.len;
  sb->data[sb->len] = 0;
}

void clear(StringBuilder *sb) {
  sb->len = 0;
  if (sb->data != s_empty) {
    sb->data[0] = 0;
  }
}

void relative_path(StringBuilder *sb, String filepath, String file) {
  clear(sb);

  u64 slash = last_of(filepath, '/');
  if (slash != (u64)-1) {
    String path = substr(filepath, 0, slash + 1);
    concat(sb, path);
  }

  concat(sb, file);
}

void format(StringBuilder *sb, const char *fmt, ...) {
  clear(sb);

  va_list args;
  va_start(args, fmt);
  i32 len = vsnprintf(nullptr, 0, fmt, args);
  va_end(args);

  if (len <= 0) {
    return;
  }

  if (len + 1 >= sb->capacity) {
    reserve(sb, len + 1);
  }

  va_start(args, fmt);
  vsnprintf(sb->data, sb->capacity, fmt, args);
  va_end(args);

  sb->len = len;
}
