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

i32 utf8_size(u8 c) {
  if (c == '\0') {
    return 0;
  }

  if ((c & 0xF8) == 0xF0) {
    return 4;
  } else if ((c & 0xF0) == 0xE0) {
    return 3;
  } else if ((c & 0xE0) == 0xC0) {
    return 2;
  } else {
    return 1;
  }
}

u32 rune_charcode(Rune r) {
  u32 charcode = 0;

  u8 c0 = r.value >> 0;
  u8 c1 = r.value >> 8;
  u8 c2 = r.value >> 16;
  u8 c3 = r.value >> 24;

  switch (utf8_size(c0)) {
  case 1: charcode = c0; break;
  case 2:
    charcode = c0 & 0x1F;
    charcode = (charcode << 6) | (c1 & 0x3F);
    break;
  case 3:
    charcode = c0 & 0x0F;
    charcode = (charcode << 6) | (c1 & 0x3F);
    charcode = (charcode << 6) | (c2 & 0x3F);
    break;
  case 4:
    charcode = c0 & 0x07;
    charcode = (charcode << 6) | (c1 & 0x3F);
    charcode = (charcode << 6) | (c2 & 0x3F);
    charcode = (charcode << 6) | (c3 & 0x3F);
    break;
  }

  return charcode;
}

static void next_rune(UTF8Iterator *it) {
  if (it->cursor == it->str.len) {
    it->rune.value = 0;
    return;
  }

  u32 rune = 0;
  char *data = it->str.data;
  i32 len = utf8_size(data[it->cursor]);
  for (i32 i = len - 1; i >= 0; i--) {
    rune <<= 8;
    rune |= (u8)(data[it->cursor + i]);
  }

  it->cursor += len;
  it->rune.value = rune;
}

UTF8Iterator &UTF8Iterator::operator++() {
  next_rune(this);
  return *this;
}

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs) {
  return lhs.str.data != rhs.str.data || lhs.str.len != rhs.str.len ||
         lhs.cursor != rhs.cursor || lhs.rune.value != rhs.rune.value;
}

UTF8Iterator begin(UTF8 utf8) {
  UTF8Iterator it = {};
  it.str = utf8.str;
  next_rune(&it);
  return it;
}

UTF8Iterator end(UTF8 utf8) { return {utf8.str, utf8.str.len, {}}; }

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
