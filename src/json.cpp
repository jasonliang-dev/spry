#include "json.h"
#include "profile.h"
#include "strings.h"

enum JSONTok : i32 {
  JSONTok_Invalid,
  JSONTok_LBrace,   // {
  JSONTok_RBrace,   // }
  JSONTok_LBracket, // [
  JSONTok_RBracket, // ]
  JSONTok_Colon,    // :
  JSONTok_Comma,    // ,
  JSONTok_True,     // true
  JSONTok_False,    // false
  JSONTok_Null,     // null
  JSONTok_String,   // "[^"]*"
  JSONTok_Number,   // [0-9]+\.?[0-9]*
  JSONTok_Error,
  JSONTok_EOF,
};

const char *json_tok_string(JSONTok tok) {
  switch (tok) {
  case JSONTok_Invalid: return "Invalid";
  case JSONTok_LBrace: return "LBrace";
  case JSONTok_RBrace: return "RBrace";
  case JSONTok_LBracket: return "LBracket";
  case JSONTok_RBracket: return "RBracket";
  case JSONTok_Colon: return "Colon";
  case JSONTok_Comma: return "Comma";
  case JSONTok_True: return "True";
  case JSONTok_False: return "False";
  case JSONTok_Null: return "Null";
  case JSONTok_String: return "String";
  case JSONTok_Number: return "Number";
  case JSONTok_Error: return "Error";
  case JSONTok_EOF: return "EOF";
  default: return "?";
  }
}

const char *json_kind_string(JSONKind kind) {
  switch (kind) {
  case JSONKind_Null: return "Null";
  case JSONKind_Object: return "Object";
  case JSONKind_Array: return "Array";
  case JSONKind_String: return "String";
  case JSONKind_Number: return "Number";
  case JSONKind_Boolean: return "Boolean";
  default: return "?";
  }
};

struct JSONToken {
  JSONTok kind;
  String str;
  u32 line;
  u32 column;
};

struct JSONScanner {
  String contents;
  JSONToken token;
  u64 begin;
  u64 end;
  u32 line;
  u32 column;
};

static char json_peek(JSONScanner *scan, u64 offset) {
  return scan->contents.data[scan->end + offset];
}

static bool json_at_end(JSONScanner *scan) {
  return scan->end == scan->contents.len;
}

static void json_next_char(JSONScanner *scan) {
  if (!json_at_end(scan)) {
    scan->end++;
    scan->column++;
  }
}

static void json_skip_whitespace(JSONScanner *scan) {
  while (true) {
    switch (json_peek(scan, 0)) {
    case '\n': scan->column = 0; scan->line++;
    case ' ':
    case '\t':
    case '\r': json_next_char(scan); break;
    default: return;
    }
  }
}

static String json_lexeme(JSONScanner *scan) {
  return substr(scan->contents, scan->begin, scan->end);
}

static JSONToken json_make_tok(JSONScanner *scan, JSONTok kind) {
  JSONToken t = {};
  t.kind = kind;
  t.str = json_lexeme(scan);
  t.line = scan->line;
  t.column = scan->column;

  scan->token = t;
  return t;
}

static JSONToken json_err_tok(JSONScanner *scan, String msg) {
  JSONToken t = {};
  t.kind = JSONTok_Error;
  t.str = msg;
  t.line = scan->line;
  t.column = scan->column;

  scan->token = t;
  return t;
}

static JSONToken json_scan_ident(Arena *a, JSONScanner *scan) {
  while (is_alpha(json_peek(scan, 0))) {
    json_next_char(scan);
  }

  JSONToken t = {};
  t.str = json_lexeme(scan);

  if (t.str == "true") {
    t.kind = JSONTok_True;
  } else if (t.str == "false") {
    t.kind = JSONTok_False;
  } else if (t.str == "null") {
    t.kind = JSONTok_Null;
  } else {
    StringBuilder sb = string_builder_make();
    string_builder_concat(&sb, "unknown identifier: '");
    string_builder_concat(&sb, t.str);
    string_builder_concat(&sb, "'");

    String s = arena_bump_string(a, string_builder_as_string(&sb));
    return json_err_tok(scan, s);
  }

  scan->token = t;
  return t;
}

static JSONToken json_scan_number(JSONScanner *scan) {
  if (json_peek(scan, 0) == '-' && is_digit(json_peek(scan, 1))) {
    json_next_char(scan); // eat '-'
  }

  while (is_digit(json_peek(scan, 0))) {
    json_next_char(scan);
  }

  if (json_peek(scan, 0) == '.' && is_digit(json_peek(scan, 1))) {
    json_next_char(scan); // eat '.'

    while (is_digit(json_peek(scan, 0))) {
      json_next_char(scan);
    }
  }

  return json_make_tok(scan, JSONTok_Number);
}

static JSONToken json_scan_string(JSONScanner *scan) {
  while (json_peek(scan, 0) != '"' && !json_at_end(scan)) {
    json_next_char(scan);
  }

  if (json_at_end(scan)) {
    return json_err_tok(scan, "unterminated string");
  }

  json_next_char(scan);
  return json_make_tok(scan, JSONTok_String);
}

static JSONToken json_scan_next(Arena *a, JSONScanner *scan) {
  json_skip_whitespace(scan);

  scan->begin = scan->end;

  if (json_at_end(scan)) {
    return json_make_tok(scan, JSONTok_EOF);
  }

  char c = json_peek(scan, 0);
  json_next_char(scan);

  if (is_alpha(c)) {
    return json_scan_ident(a, scan);
  }

  if (is_digit(c) || (c == '-' && is_digit(json_peek(scan, 0)))) {
    return json_scan_number(scan);
  }

  if (c == '"') {
    return json_scan_string(scan);
  }

  switch (c) {
  case '{': return json_make_tok(scan, JSONTok_LBrace);
  case '}': return json_make_tok(scan, JSONTok_RBrace);
  case '[': return json_make_tok(scan, JSONTok_LBracket);
  case ']': return json_make_tok(scan, JSONTok_RBracket);
  case ':': return json_make_tok(scan, JSONTok_Colon);
  case ',': return json_make_tok(scan, JSONTok_Comma);
  }

  String msg = tmp_fmt("unexpected character: '%c' (%d)", c, (int)c);
  String s = arena_bump_string(a, msg);
  return json_err_tok(scan, s);
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out);

static String json_parse_object(Arena *a, JSONScanner *scan, JSONObject **out) {
  PROFILE_FUNC();

  JSONObject *obj = nullptr;

  json_scan_next(a, scan); // eat brace

  while (true) {
    if (scan->token.kind == JSONTok_RBrace) {
      *out = obj;
      json_scan_next(a, scan);
      return {};
    }

    String err = {};

    JSON key = {};
    err = json_parse_next(a, scan, &key);
    if (err.data != nullptr) {
      return err;
    }

    if (key.kind != JSONKind_String) {
      String msg = tmp_fmt("expected string as object key on line: %d. got: %s",
                           (i32)scan->token.line, json_kind_string(key.kind));
      return arena_bump_string(a, msg);
    }

    if (scan->token.kind != JSONTok_Colon) {
      String msg =
          tmp_fmt("expected colon on line: %d. got %s", (i32)scan->token.line,
                  json_tok_string(scan->token.kind));
      return arena_bump_string(a, msg);
    }

    json_scan_next(a, scan);

    JSON value = {};
    err = json_parse_next(a, scan, &value);
    if (err.data != nullptr) {
      return err;
    }

    JSONObject *entry = (JSONObject *)arena_bump(a, sizeof(JSONObject));
    entry->next = obj;
    entry->key = fnv1a(key.string.data, key.string.len);
    entry->value = value;

    obj = entry;

    if (scan->token.kind == JSONTok_Comma) {
      json_scan_next(a, scan);
    }
  }
}

static String json_parse_array(Arena *a, JSONScanner *scan, JSONArray **out) {
  PROFILE_FUNC();

  JSONArray *arr = nullptr;

  json_scan_next(a, scan); // eat bracket

  while (true) {
    if (scan->token.kind == JSONTok_RBracket) {
      *out = arr;
      json_scan_next(a, scan);
      return {};
    }

    JSON value = {};
    String err = json_parse_next(a, scan, &value);
    if (err.data != nullptr) {
      return err;
    }

    JSONArray *el = (JSONArray *)arena_bump(a, sizeof(JSONArray));
    el->next = arr;
    el->value = value;
    el->count = 0;

    if (arr != nullptr) {
      el->count = arr->count + 1;
    }

    arr = el;

    if (scan->token.kind == JSONTok_Comma) {
      json_scan_next(a, scan);
    }
  }
}

static JSONArray *reverse_list(JSONArray *head) {
  JSONArray *prev = nullptr;
  while (head != nullptr) {
    JSONArray *next = head->next;

    head->next = prev;
    prev = head;
    head = next;
  }
  return prev;
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out) {
  switch (scan->token.kind) {
  case JSONTok_LBrace: {
    out->kind = JSONKind_Object;
    return json_parse_object(a, scan, &out->object);
  }
  case JSONTok_LBracket: {
    out->kind = JSONKind_Array;

    JSONArray *arr = nullptr;
    String res = json_parse_array(a, scan, &arr);
    if (res.len != 0) {
      return res;
    }

    out->array = reverse_list(arr);
    return res;
  }
  case JSONTok_String: {
    out->kind = JSONKind_String;
    out->string = substr(scan->token.str, 1, scan->token.str.len - 1);
    json_scan_next(a, scan);
    return {};
  }
  case JSONTok_Number: {
    out->kind = JSONKind_Number;
    out->number = string_to_double(scan->token.str);
    json_scan_next(a, scan);
    return {};
  }
  case JSONTok_True: {
    out->kind = JSONKind_Boolean;
    out->boolean = true;
    json_scan_next(a, scan);
    return {};
  }
  case JSONTok_False: {
    out->kind = JSONKind_Boolean;
    out->boolean = false;
    json_scan_next(a, scan);
    return {};
  }
  case JSONTok_Null: {
    out->kind = JSONKind_Null;
    json_scan_next(a, scan);
    return {};
  }
  default:
    String msg = tmp_fmt("unknown json token: %s on line %d:%d",
                         json_tok_string(scan->token.kind),
                         (i32)scan->token.line, (i32)scan->token.column);
    return arena_bump_string(a, msg);
  }
}

void json_parse(JSONDocument *out, String contents) {
  PROFILE_FUNC();

  out->arena = {};

  JSONScanner scan = {};
  scan.contents = contents;
  scan.line = 1;

  json_scan_next(&out->arena, &scan);

  String err = json_parse_next(&out->arena, &scan, &out->root);
  if (err.data != nullptr) {
    out->error = err;
    return;
  }

  if (scan.token.kind != JSONTok_EOF) {
    out->error = "expected EOF";
    return;
  }
}

void json_trash(JSONDocument *doc) {
  PROFILE_FUNC();
  arena_trash(&doc->arena);
}

static void json_read_error(JSON *json) {
  json->had_error = true;
  if (json->parent) {
    json_read_error(json->parent);
  }
}

static bool json_is_bad(JSON *json) {
  return json == nullptr || json->had_error;
}

JSON *json_lookup(JSON *obj, String key) {
  if (json_is_bad(obj)) {
    return nullptr;
  }

  if (obj->kind == JSONKind_Object) {
    for (JSONObject *o = obj->object; o != nullptr; o = o->next) {
      if (o->key == fnv1a(key)) {
        return &o->value;
      }
    }
  }

  json_read_error(obj);
  return nullptr;
}

JSON *json_index(JSON *arr, i32 index) {
  if (json_is_bad(arr)) {
    return nullptr;
  }

  if (arr->kind == JSONKind_Array) {
    for (JSONArray *a = arr->array; a != nullptr; a = a->next) {
      if (a->count == index) {
        return &a->value;
      }
    }
  }

  json_read_error(arr);
  return nullptr;
}

JSONObject *json_object(JSON *json) {
  if (json_is_bad(json)) {
    return {};
  } else if (json->kind != JSONKind_Object) {
    json_read_error(json);
    return {};
  } else {
    for (JSONObject *o = json->object; o != nullptr; o = o->next) {
      o->value.parent = json;
    }
    return json->object;
  }
}

JSONArray *json_array(JSON *json) {
  if (json_is_bad(json)) {
    return {};
  } else if (json->kind != JSONKind_Array) {
    json_read_error(json);
    return {};
  } else {
    for (JSONArray *a = json->array; a != nullptr; a = a->next) {
      a->value.parent = json;
    }
    return json->array;
  }
}

String json_string(JSON *json) {
  if (json_is_bad(json)) {
    return {};
  } else if (json->kind != JSONKind_String) {
    json_read_error(json);
    return {};
  } else {
    return json->string;
  }
}

double json_number(JSON *json) {
  if (json_is_bad(json)) {
    return 0;
  } else if (json->kind != JSONKind_Number) {
    json_read_error(json);
    return 0;
  } else {
    return json->number;
  }
}

static void json_write_string(StringBuilder *sb, JSON *json, i32 level) {
  switch (json->kind) {
  case JSONKind_Object: {
    string_builder_concat(sb, "{\n");
    for (JSONObject *o = json->object; o != nullptr; o = o->next) {
      for (i32 i = 0; i <= level; i++) {
        string_builder_concat(sb, "  ");
      }
      char buf[64];
      snprintf(buf, sizeof(buf), "%llu", (unsigned long long)o->key);
      string_builder_concat(sb, buf);

      json_write_string(sb, &o->value, level + 1);
      string_builder_concat(sb, ",\n");
    }
    for (i32 i = 0; i < level; i++) {
      string_builder_concat(sb, "  ");
    }
    string_builder_concat(sb, "}");
    break;
  }
  case JSONKind_Array: {
    string_builder_concat(sb, "[\n");
    for (JSONArray *a = json->array; a != nullptr; a = a->next) {
      for (i32 i = 0; i <= level; i++) {
        string_builder_concat(sb, "  ");
      }
      json_write_string(sb, &a->value, level + 1);
      string_builder_concat(sb, ",\n");
    }
    for (i32 i = 0; i < level; i++) {
      string_builder_concat(sb, "  ");
    }
    string_builder_concat(sb, "]");
    break;
  }
  case JSONKind_String: {
    string_builder_concat(sb, "\"");
    string_builder_concat(sb, json->string);
    string_builder_concat(sb, "\"");
    break;
  }
  case JSONKind_Number: {
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", json->number);
    string_builder_concat(sb, buf);
    break;
  }
  case JSONKind_Boolean: {
    string_builder_concat(sb, json->boolean ? "true" : "false");
    break;
  }
  case JSONKind_Null: {
    string_builder_concat(sb, "null");
    break;
  }
  default: break;
  }
}

void json_write_string(StringBuilder *sb, JSON *json) {
  json_write_string(sb, json, 0);
}

void json_print(JSON *json) {
  StringBuilder sb = string_builder_make();
  defer(string_builder_trash(&sb));
  json_write_string(&sb, json);
  printf("%s\n", sb.data);
}