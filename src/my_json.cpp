#include "my_json.h"

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
  case JsonKind_Null: return "Null";
  case JsonKind_Object: return "Object";
  case JsonKind_Array: return "Array";
  case JsonKind_String: return "String";
  case JsonKind_Number: return "Number";
  case JsonKind_Boolean: return "Boolean";
  default: return "?";
  }
};

typedef struct {
  JSONTok kind;
  String str;
  u32 line;
  u32 column;
} JSONToken;

typedef struct {
  Array<String> strings;
  String contents;
  JSONToken token;
  u64 begin;
  u64 end;
  u32 line;
  u32 column;
} JSONScanner;

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

static JSONToken json_scan_ident(JSONScanner *scan) {
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

    String s = string_builder_as_string(&sb);
    array_push(&scan->strings, s);
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

static JSONToken json_scan_next(JSONScanner *scan) {
  json_skip_whitespace(scan);

  scan->begin = scan->end;

  if (json_at_end(scan)) {
    return json_make_tok(scan, JSONTok_EOF);
  }

  char c = json_peek(scan, 0);
  json_next_char(scan);

  if (is_alpha(c)) {
    return json_scan_ident(scan);
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

  String s = str_format("unexpected character: '%c' (%d)", c, (int)c);
  array_push(&scan->strings, s);
  return json_err_tok(scan, s);
}

static String json_parse_next(JSONScanner *scan, JSON *out);

static String json_parse_object(JSONScanner *scan, HashMap<JSON> *out) {
  HashMap<JSON> obj = {};

  json_scan_next(scan); // eat brace

  while (true) {
    if (scan->token.kind == JSONTok_RBrace) {
      *out = obj;
      json_scan_next(scan);
      return {};
    }

    String err = {};

    JSON key = {};
    err = json_parse_next(scan, &key);
    if (err.len != 0) {
      return err;
    }

    if (key.kind != JsonKind_String) {
      String s =
          str_format("expected string as object key on line: %d. got: %s",
                     (i32)scan->token.line, json_kind_string(key.kind));
      array_push(&scan->strings, s);
      return s;
    }

    if (scan->token.kind != JSONTok_Colon) {
      String s =
          str_format("expected colon on line: %d. got %s",
                     (i32)scan->token.line, json_tok_string(scan->token.kind));
      array_push(&scan->strings, s);
      return s;
    }

    json_scan_next(scan);

    JSON value = {};
    err = json_parse_next(scan, &value);
    if (err.len != 0) {
      return err;
    }

    obj[fnv1a(key.string)] = value;

    if (scan->token.kind == JSONTok_Comma) {
      json_scan_next(scan);
    }
  }
}

static String json_parse_array(JSONScanner *scan, Array<JSON> *out) {
  Array<JSON> arr = {};

  json_scan_next(scan); // eat bracket

  while (true) {
    if (scan->token.kind == JSONTok_RBracket) {
      *out = arr;
      json_scan_next(scan);
      return {};
    }

    JSON value = {};
    String err = json_parse_next(scan, &value);
    if (err.len != 0) {
      return err;
    }

    array_push(&arr, value);

    if (scan->token.kind == JSONTok_Comma) {
      json_scan_next(scan);
    }
  }
}

static String json_parse_next(JSONScanner *scan, JSON *out) {
  switch (scan->token.kind) {
  case JSONTok_LBrace: {
    out->kind = JsonKind_Object;
    return json_parse_object(scan, &out->object);
  }
  case JSONTok_LBracket: {
    out->kind = JsonKind_Array;
    return json_parse_array(scan, &out->array);
  }
  case JSONTok_String: {
    out->kind = JsonKind_String;
    out->string = substr(scan->token.str, 1, scan->token.str.len - 1);
    json_scan_next(scan);
    return {};
  }
  case JSONTok_Number: {
    out->kind = JsonKind_Number;
    out->number = string_to_double(scan->token.str);
    json_scan_next(scan);
    return {};
  }
  case JSONTok_True: {
    out->kind = JsonKind_Boolean;
    out->boolean = true;
    json_scan_next(scan);
    return {};
  }
  case JSONTok_False: {
    out->kind = JsonKind_Boolean;
    out->boolean = false;
    json_scan_next(scan);
    return {};
  }
  case JSONTok_Null: {
    out->kind = JsonKind_Null;
    json_scan_next(scan);
    return {};
  }
  default:
    String s = str_format("unknown json token: %s on line %d:%d",
                          json_tok_string(scan->token.kind),
                          (i32)scan->token.line, (i32)scan->token.column);
    array_push(&scan->strings, s);
    return s;
  }
}

String json_parse(JSON *json, String contents) {
  JSONScanner scan = {};
  scan.contents = contents;
  scan.line = 1;

  defer({
    for (String s : scan.strings) {
      mem_free(s.data);
    }
    array_trash(&scan.strings);
  });

  json_scan_next(&scan);

  String err = json_parse_next(&scan, json);
  if (err.len != 0) {
    return err;
  }

  if (scan.token.kind != JSONTok_EOF) {
    return "expected EOF";
  }

  return {};
}

void json_trash(JSON *json) {
  switch (json->kind) {
  case JsonKind_Object:
    for (auto [k, v] : json->object) {
      json_trash(v);
    }
    hashmap_trash(&json->object);
    break;
  case JsonKind_Array:
    for (JSON &value : json->array) {
      json_trash(&value);
    }
    array_trash(&json->array);
    break;
  default: break;
  }
}

JSON json_lookup(JSON obj, String key) {
  if (obj.kind == JsonKind_Object) {
    JSON *value = hashmap_get(&obj.object, fnv1a(key));
    if (value != nullptr) {
      return *value;
    }
  }

  return {};
}

JSON json_index(JSON arr, i32 index) {
  if (arr.kind == JsonKind_Array) {
    return arr.array[index];
  } else {
    return {};
  }
}

HashMap<JSON> json_object(JSON json) {
  if (json.kind == JsonKind_Object) {
    return json.object;
  } else {
    return {};
  }
}

Array<JSON> json_array(JSON json) {
  if (json.kind == JsonKind_Array) {
    return json.array;
  } else {
    return {};
  }
}

String json_string(JSON json) {
  if (json.kind == JsonKind_String) {
    return json.string;
  } else {
    return {};
  }
}

double json_number(JSON json) {
  if (json.kind == JsonKind_Number) {
    return json.number;
  } else {
    return 0;
  }
}

static void json_write_string(StringBuilder *sb, JSON json, i32 level) {
  switch (json.kind) {
  case JsonKind_Object: {
    string_builder_concat(sb, "{\n");
    for (auto [k, v] : json.object) {
      for (i32 i = 0; i <= level; i++) {
        string_builder_concat(sb, "  ");
      }
      String s = str_format("%d: ", (i32)k);
      defer(mem_free(s.data));
      string_builder_concat(sb, s);

      json_write_string(sb, *v, level + 1);
      string_builder_concat(sb, ",\n");
    }
    for (i32 i = 0; i < level; i++) {
      string_builder_concat(sb, "  ");
    }
    string_builder_concat(sb, "}");
    break;
  }
  case JsonKind_Array: {
    string_builder_concat(sb, "[\n");
    for (JSON &value : json.array) {
      for (i32 i = 0; i <= level; i++) {
        string_builder_concat(sb, "  ");
      }
      json_write_string(sb, value, level + 1);
      string_builder_concat(sb, ",\n");
    }
    for (i32 i = 0; i < level; i++) {
      string_builder_concat(sb, "  ");
    }
    string_builder_concat(sb, "]");
    break;
  }
  case JsonKind_String: {
    string_builder_concat(sb, "\"");
    string_builder_concat(sb, json.string);
    string_builder_concat(sb, "\"");
    break;
  }
  case JsonKind_Number: {
    String s = str_format("%d", (i32)json.number);
    defer(mem_free(s.data));
    string_builder_concat(sb, s);
    break;
  }
  case JsonKind_Boolean: {
    string_builder_concat(sb, json.boolean ? "true" : "false");
    break;
  }
  case JsonKind_Null: {
    string_builder_concat(sb, "null");
    break;
  }
  default: break;
  }
}

void json_write_string(StringBuilder *sb, JSON json) {
  json_write_string(sb, json, 0);
}

void json_print(JSON json) {
  StringBuilder sb = string_builder_make();
  defer(string_builder_trash(&sb));
  json_write_string(&sb, json);
  printf("%s\n", sb.data);
}