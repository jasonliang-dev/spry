#include "scanner.h"
#include "strings.h"

Scanner::Scanner(String str) {
  data = str.data;
  len = str.len;
  pos = 0;
  end = 0;
}

static void advance(Scanner *s) { s->end += utf8_size(s->data[s->end]); }
static bool at_end(Scanner *s) { return s->end >= s->len; }

static Rune peek(Scanner *s) {
  if (at_end(s)) {
    return {0};
  } else {
    return rune_from_string(&s->data[s->end]);
  }
}

static void skip_whitespace(Scanner *s) {
  while (peek(s).is_whitespace() && !at_end(s)) {
    advance(s);
  }
}

String Scanner::next_string() {
  skip_whitespace(this);
  pos = end;

  if (at_end(this)) {
    return "";
  }

  while (!peek(this).is_whitespace() && !at_end(this)) {
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
  if (peek(this).value == '-') {
    sign = -1;
    advance(this);
  }

  i32 num = 0;
  while (peek(this).is_digit()) {
    num *= 10;
    num += peek(this).value - '0';
    advance(this);
  }

  return num * sign;
}