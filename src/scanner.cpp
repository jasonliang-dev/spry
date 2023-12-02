#include "scanner.h"

Scanner::Scanner(String str) {
  data = str.data;
  len = str.len;
  pos = 0;
  end = 0;
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

static void skip_whitespace(Scanner *s) {
  while (is_whitespace(peek(s)) && peek(s) != 0) {
    advance(s);
  }
}

String Scanner::next_string() {
  skip_whitespace(this);
  pos = end;

  if (at_end(this)) {
    return "";
  }

  while (!is_whitespace(peek(this)) && peek(this) != 0) {
    advance(this);
  }

  return {&data[pos], end - pos};
}

i32 Scanner::next_int() {
  skip_whitespace(this);
  pos = end;

  if (at_end(this)) {
    return 0;
  }

  i32 sign = 1;
  if (peek(this) == '-') {
    sign = -1;
    advance(this);
  }

  i32 num = 0;
  while (is_digit(peek(this))) {
    num *= 10;
    num += peek(this) - '0';
    advance(this);
  }

  return num * sign;
}