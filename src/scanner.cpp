#include "scanner.h"

Scanner make_scanner(String str) {
  Scanner s = {};
  s.data = str.data;
  s.len = str.len;
  s.pos = 0;
  s.end = 0;
  return s;
}

static void advance(Scanner *s) { s->end++; }
static bool at_end(Scanner *s) { return s->end == s->len; }

static char peek(Scanner *s) {
  if (at_end(s)) {
    return 0;
  } else {
    return s->data[s->end];
  }
}

static bool is_whitespace(char c) {
  switch (c) {
  case '\n':
  case '\r':
  case '\t':
  case ' ': return true;
  }
  return false;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static void skip_whitespace(Scanner *s) {
  while (is_whitespace(peek(s)) && peek(s) != 0) {
    advance(s);
  }
}

String scan_next_string(Scanner *s) {
  skip_whitespace(s);
  s->pos = s->end;

  if (at_end(s)) {
    return "";
  }

  while (!is_whitespace(peek(s)) && peek(s) != 0) {
    advance(s);
  }

  return {&s->data[s->pos], s->end - s->pos};
}

i32 scan_next_int(Scanner *s) {
  skip_whitespace(s);
  s->pos = s->end;

  if (at_end(s)) {
    return 0;
  }

  i32 sign = 1;
  if (peek(s) == '-') {
    sign = -1;
    advance(s);
  }

  i32 num = 0;
  while (is_digit(peek(s))) {
    num *= 10;
    num += peek(s) - '0';
    advance(s);
  }

  return num * sign;
}