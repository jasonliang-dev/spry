#include "json.h"
#include "arena.h"
#include "hash_map.h"
#include "luax.h"
#include "prelude.h"
#include "profile.h"
#include "strings.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

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
  return scan->contents.substr(scan->begin, scan->end);
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
    StringBuilder sb = {};
    defer(sb.trash());

    String s = String(sb << "unknown identifier: '" << t.str << "'");
    return json_err_tok(scan, a->bump_string(s));
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
  String s = a->bump_string(msg);
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
      return a->bump_string(msg);
    }

    if (scan->token.kind != JSONTok_Colon) {
      String msg =
          tmp_fmt("expected colon on line: %d. got %s", (i32)scan->token.line,
                  json_tok_string(scan->token.kind));
      return a->bump_string(msg);
    }

    json_scan_next(a, scan);

    JSON value = {};
    err = json_parse_next(a, scan, &value);
    if (err.data != nullptr) {
      return err;
    }

    JSONObject *entry = (JSONObject *)a->bump(sizeof(JSONObject));
    entry->next = obj;
    entry->hash = fnv1a(key.string);
    entry->key = key.string;
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

    JSONArray *el = (JSONArray *)a->bump(sizeof(JSONArray));
    el->next = arr;
    el->value = value;
    el->index = 0;

    if (arr != nullptr) {
      el->index = arr->index + 1;
    }

    arr = el;

    if (scan->token.kind == JSONTok_Comma) {
      json_scan_next(a, scan);
    }
  }
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out) {
  switch (scan->token.kind) {
  case JSONTok_LBrace: {
    out->kind = JSONKind_Object;
    return json_parse_object(a, scan, &out->object);
  }
  case JSONTok_LBracket: {
    out->kind = JSONKind_Array;
    return json_parse_array(a, scan, &out->array);
  }
  case JSONTok_String: {
    out->kind = JSONKind_String;
    out->string = scan->token.str.substr(1, scan->token.str.len - 1);
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
  case JSONTok_Error: {
    StringBuilder sb = {};
    defer(sb.trash());

    sb << scan->token.str
       << tmp_fmt(" on line %d:%d", (i32)scan->token.line,
                  (i32)scan->token.column);

    return a->bump_string(String(sb));
  }
  default: {
    String msg = tmp_fmt("unknown json token: %s on line %d:%d",
                         json_tok_string(scan->token.kind),
                         (i32)scan->token.line, (i32)scan->token.column);
    return a->bump_string(msg);
  }
  }
}

void JSONDocument::parse(String contents) {
  PROFILE_FUNC();

  arena = {};

  JSONScanner scan = {};
  scan.contents = contents;
  scan.line = 1;

  json_scan_next(&arena, &scan);

  String err = json_parse_next(&arena, &scan, &root);
  if (err.data != nullptr) {
    error = err;
    return;
  }

  if (scan.token.kind != JSONTok_EOF) {
    error = "expected EOF";
    return;
  }
}

void JSONDocument::trash() {
  PROFILE_FUNC();
  arena.trash();
}

JSON JSON::lookup(String key, bool *ok) {
  if (*ok && kind == JSONKind_Object) {
    for (JSONObject *o = object; o != nullptr; o = o->next) {
      if (o->hash == fnv1a(key)) {
        return o->value;
      }
    }
  }

  *ok = false;
  return {};
}

JSON JSON::index(i32 i, bool *ok) {
  if (*ok && kind == JSONKind_Array) {
    for (JSONArray *a = array; a != nullptr; a = a->next) {
      if (a->index == i) {
        return a->value;
      }
    }
  }

  *ok = false;
  return {};
}

JSONObject *JSON::as_object(bool *ok) {
  if (*ok && kind == JSONKind_Object) {
    return object;
  }

  *ok = false;
  return {};
}

JSONArray *JSON::as_array(bool *ok) {
  if (*ok && kind == JSONKind_Array) {
    return array;
  }

  *ok = false;
  return {};
}

String JSON::as_string(bool *ok) {
  if (*ok && kind == JSONKind_String) {
    return string;
  }

  *ok = false;
  return {};
}

double JSON::as_number(bool *ok) {
  if (*ok && kind == JSONKind_Number) {
    return number;
  }

  *ok = false;
  return {};
}

JSONObject *JSON::lookup_object(String key, bool *ok) {
  return lookup(key, ok).as_object(ok);
}

JSONArray *JSON::lookup_array(String key, bool *ok) {
  return lookup(key, ok).as_array(ok);
}

String JSON::lookup_string(String key, bool *ok) {
  return lookup(key, ok).as_string(ok);
}

double JSON::lookup_number(String key, bool *ok) {
  return lookup(key, ok).as_number(ok);
}

double JSON::index_number(i32 i, bool *ok) {
  return index(i, ok).as_number(ok);
}

static void json_write_string(StringBuilder &sb, JSON *json, i32 level) {
  switch (json->kind) {
  case JSONKind_Object: {
    sb << "{\n";
    for (JSONObject *o = json->object; o != nullptr; o = o->next) {
      sb.concat("  ", level);
      sb << o->key;
      json_write_string(sb, &o->value, level + 1);
      sb << ",\n";
    }
    sb.concat("  ", level - 1);
    for (i32 i = 0; i < level; i++) {
      sb << "  ";
    }
    sb << "}";
    break;
  }
  case JSONKind_Array: {
    sb << "[\n";
    for (JSONArray *a = json->array; a != nullptr; a = a->next) {
      sb.concat("  ", level);
      json_write_string(sb, &a->value, level + 1);
      sb << ",\n";
    }
    sb.concat("  ", level - 1);
    sb << "]";
    break;
  }
  case JSONKind_String: sb << "\"" << json->string << "\""; break;
  case JSONKind_Number: sb << tmp_fmt("%g", json->number); break;
  case JSONKind_Boolean: sb << (json->boolean ? "true" : "false"); break;
  case JSONKind_Null: sb << "null"; break;
  default: break;
  }
}

void json_write_string(StringBuilder *sb, JSON *json) {
  json_write_string(*sb, json, 1);
}

void json_print(JSON *json) {
  StringBuilder sb = {};
  defer(sb.trash());
  json_write_string(&sb, json);
  printf("%s\n", sb.data);
}

void json_to_lua(lua_State *L, JSON *json) {
  switch (json->kind) {
  case JSONKind_Object: {
    lua_newtable(L);
    for (JSONObject *o = json->object; o != nullptr; o = o->next) {
      lua_pushlstring(L, o->key.data, o->key.len);
      json_to_lua(L, &o->value);
      lua_rawset(L, -3);
    }
    break;
  }
  case JSONKind_Array: {
    lua_newtable(L);
    for (JSONArray *a = json->array; a != nullptr; a = a->next) {
      json_to_lua(L, &a->value);
      lua_rawseti(L, -2, a->index + 1);
    }
    break;
  }
  case JSONKind_String: {
    lua_pushlstring(L, json->string.data, json->string.len);
    break;
  }
  case JSONKind_Number: {
    lua_pushnumber(L, json->number);
    break;
  }
  case JSONKind_Boolean: {
    lua_pushboolean(L, json->boolean);
    break;
  }
  case JSONKind_Null: {
    lua_pushnil(L);
    break;
  }
  default: break;
  }
}

static void lua_to_json_string(StringBuilder &sb, lua_State *L,
                               HashMap<bool> *visited, String *err) {
  if (err->len != 0) {
    return;
  }

  i32 top = lua_gettop(L);
  switch (lua_type(L, top)) {
  case LUA_TTABLE: {
    uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

    bool *visit = nullptr;
    bool exist = visited->find_or_insert(ptr, &visit);
    if (exist && *visit) {
      *err = "table has cycles";
      return;
    }

    *visit = true;

    lua_pushnil(L);
    if (lua_next(L, -2) == 0) {
      sb << "[]";
      return;
    }

    i32 key_type = lua_type(L, -2);

    if (key_type == LUA_TNUMBER) {
      sb << "[";
      lua_to_json_string(sb, L, visited, err);

      i32 len = luax_len(L, top);
      assert(len > 0);
      i32 i = 1;
      for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
        if (lua_type(L, -2) != LUA_TNUMBER) {
          lua_pop(L, -2);
          *err = "expected all keys to be numbers";
          return;
        }

        sb << ",";
        lua_to_json_string(sb, L, visited, err);
        i++;
      }
      sb << "]";

      if (i != len) {
        *err = "array is not continuous";
        return;
      }
    } else if (key_type == LUA_TSTRING) {
      sb << "{";

      lua_pushvalue(L, -2);
      lua_to_json_string(sb, L, visited, err);
      lua_pop(L, 1);
      sb << ":";
      lua_to_json_string(sb, L, visited, err);

      for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
        sb << ",";
        if (lua_type(L, -2) != LUA_TSTRING) {
          lua_pop(L, -2);
          *err = "expected all keys to be strings";
          return;
        }

        lua_pushvalue(L, -2);
        lua_to_json_string(sb, L, visited, err);
        lua_pop(L, 1);
        sb << ":";
        lua_to_json_string(sb, L, visited, err);
      }
      sb << "}";
    } else {
      lua_pop(L, 2); // key, value
      *err = "expected table keys to be strings or numbers";
      return;
    }

    visited->unset(ptr);
    break;
  }
  case LUA_TNIL: sb << "null"; break;
  case LUA_TNUMBER: sb << tmp_fmt("%g", lua_tonumber(L, top)); break;
  case LUA_TSTRING: sb << "\"" << luax_check_string(L, top) << "\""; break;
  case LUA_TBOOLEAN: sb << (lua_toboolean(L, top) ? "true" : "false"); break;
  default: *err = "type is not serializable";
  }
}

void lua_to_json_string(lua_State *L, i32 arg, String *contents, String *err) {
  StringBuilder sb = {};
  defer(sb.trash());

  HashMap<bool> visited = {};
  defer(visited.trash());

  lua_pushvalue(L, arg);
  lua_to_json_string(sb, L, &visited, err);
  lua_pop(L, 1);

  if (err->len != 0) {
    sb.trash();
  }

  *contents = String(sb);
}
