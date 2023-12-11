#include "strings.h"
#include <stdarg.h>
#include <stdio.h>

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

SplitLinesIterator SplitLines::begin() {
  char *data = str.data;
  u64 end = 0;
  while (data[end] != '\n' && data[end] != 0) {
    end++;
  }

  String view = {str.data, end};
  return {str, view};
}

SplitLinesIterator SplitLines::end() {
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

u32 Rune::charcode() {
  u32 charcode = 0;

  u8 c0 = value >> 0;
  u8 c1 = value >> 8;
  u8 c2 = value >> 16;
  u8 c3 = value >> 24;

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

UTF8Iterator UTF8::begin() {
  UTF8Iterator it = {};
  it.str = str;
  next_rune(&it);
  return it;
}

UTF8Iterator UTF8::end() { return {str, str.len, {}}; }

static char s_empty[1] = {0};

StringBuilder::StringBuilder() {
  data = s_empty;
  len = 0;
  capacity = 0;
}

void StringBuilder::trash() {
  if (data != s_empty) {
    mem_free(data);
  }
}

void StringBuilder::reserve(u64 cap) {
  if (cap > capacity) {
    char *buf = (char *)mem_alloc(cap);
    memset(buf, 0, cap);
    memcpy(buf, data, len);

    if (data != s_empty) {
      mem_free(data);
    }

    data = buf;
    capacity = cap;
  }
}

void StringBuilder::clear() {
  len = 0;
  if (data != s_empty) {
    data[0] = 0;
  }
}

void StringBuilder::swap_filename(String filepath, String file) {
  clear();

  u64 slash = filepath.last_of('/');
  if (slash != (u64)-1) {
    String path = filepath.substr(0, slash + 1);
    *this << path;
  }

  *this << file;
}

void StringBuilder::concat(String str, i32 times) {
  for (i32 i = 0; i < times; i++) {
    *this << str;
  }
}

StringBuilder &StringBuilder::operator<<(String str) {
  u64 desired = len + str.len + 1;
  u64 cap = capacity;

  if (desired >= cap) {
    u64 growth = cap > 0 ? cap * 2 : 8;
    if (growth <= desired) {
      growth = desired;
    }

    reserve(growth);
  }

  memcpy(&data[len], str.data, str.len);
  len += str.len;
  data[len] = 0;
  return *this;
}

StringBuilder::operator String() { return {data, len}; }

String str_fmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  i32 len = vsnprintf(nullptr, 0, fmt, args);
  va_end(args);

  if (len > 0) {
    char *data = (char *)mem_alloc(len + 1);
    va_start(args, fmt);
    vsnprintf(data, len + 1, fmt, args);
    va_end(args);
    return {data, (u64)len};
  }

  return {};
}

String tmp_fmt(const char *fmt, ...) {
  static char s_buf[1024] = {};

  va_list args;
  va_start(args, fmt);
  i32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
  va_end(args);
  return {s_buf, (u64)len};
}

double string_to_double(String str) {
  double n = 0;
  double sign = 1;

  if (str.len == 0) {
    return n;
  }

  u64 i = 0;
  if (str.data[0] == '-' && str.len > 1 && is_digit(str.data[1])) {
    i++;
    sign = -1;
  }

  while (i < str.len) {
    if (!is_digit(str.data[i])) {
      break;
    }

    n = n * 10 + (str.data[i] - '0');
    i++;
  }

  if (i < str.len && str.data[i] == '.') {
    i++;
    double place = 10;
    while (i < str.len) {
      if (!is_digit(str.data[i])) {
        break;
      }

      n += (str.data[i] - '0') / place;
      place *= 10;
      i++;
    }
  }

  return n * sign;
}