#include "array.h"
#include "deps/utest.h"
#include "hash_map.h"
#include "strings.cpp"

// extern(prelude.h)
Allocator g_allocator = debug_allocator();

// prelude

UTEST(prelude, string_null_term) {
  String str = "Hello World!"_str;
  ASSERT_EQ(str.data[str.len], 0);
}

UTEST(prelude, string_lines) {
  String str = "line1\nl2\nthis is a very long line 3"_str;
  SplitLinesIterator it = begin(SplitLines(str));

  ASSERT_EQ(*it, "line1"_str);
  ++it;
  ASSERT_EQ(*it, "l2"_str);
  ++it;
  ASSERT_EQ(*it, "this is a very long line 3"_str);
  ++it;
}

UTEST(prelude, split_lines_non_null_term) {
  String str =
      "line1\nl2\nline 3 some extra stuff that won't be in the string"_str;
  str.len = 15;

  SplitLinesIterator it = begin(SplitLines(str));

  ASSERT_EQ(*it, "line1"_str);
  ++it;
  ASSERT_EQ(*it, "l2"_str);
  ++it;
  ASSERT_EQ(*it, "line 3"_str);
  ++it;
  ASSERT_EQ(*it, ""_str);
  ++it;
}

// array

UTEST(array, empty) {
  Array<int> arr = {};
  drop(&arr);
}

UTEST(array, push) {
  Array<int> arr = {};
  defer(drop(&arr));

  push(&arr, 1);
  push(&arr, 2);
  push(&arr, 3);

  ASSERT_EQ(arr.len, 3);

  ASSERT_EQ(arr[0], 1);
  ASSERT_EQ(arr[1], 2);
  ASSERT_EQ(arr[2], 3);
}

UTEST(array, pop) {
  Array<int> arr = {};
  defer(drop(&arr));

  push(&arr, 1);
  push(&arr, 2);

  ASSERT_EQ(arr.len, 2);

  ASSERT_EQ(arr[0], 1);
  ASSERT_EQ(arr[1], 2);
}

UTEST(array, begin_end) {
  Array<int> arr = {};
  defer(drop(&arr));

  for (i32 i = 0; i < 1000; i++) {
    push(&arr, i);
  }

  i32 i = 0;
  for (int n : arr) {
    ASSERT_EQ(n, arr[i]);
    ASSERT_EQ(n, i);
    i++;
  }
}

// hash map

UTEST(hash_map, empty) {
  HashMap<String> map;
  drop(&map);
}

UTEST(hash_map, insert) {
  HashMap<String> map;
  defer(drop(&map));

  map[10] = "ten"_str;
  map[20] = "twenty"_str;
  map[30] = "thirty"_str;
}

UTEST(hash_map, lookup) {
  HashMap<String> map;
  defer(drop(&map));

  map[10] = "ten"_str;
  map[20] = "twenty"_str;
  map[30] = "thirty"_str;

  String *ten = get(&map, 10);
  ASSERT_NE(ten, nullptr);
  ASSERT_STREQ(ten->data, "ten");

  String *twenty = get(&map, 20);
  ASSERT_NE(twenty, nullptr);
  ASSERT_STREQ(twenty->data, "twenty");

  String *thirty = get(&map, 30);
  ASSERT_NE(thirty, nullptr);
  ASSERT_STREQ(thirty->data, "thirty");
}

UTEST(hash_map, unset) {
  HashMap<String> map;
  defer(drop(&map));

  map[1] = "one"_str;
  map[2] = "two"_str;
  map[3] = "three"_str;

  unset(&map, 2);

  ASSERT_NE(get(&map, 1), nullptr);
  ASSERT_EQ(get(&map, 2), nullptr);
  ASSERT_NE(get(&map, 3), nullptr);
}

UTEST(hash_map, iterator) {
  HashMap<String> map;
  defer(drop(&map));

  String one = "one"_str;
  String two = "two"_str;
  String three = "three"_str;
  map[1] = one;
  map[2] = two;
  map[3] = three;

  for (auto [k, v] : map) {
    if (k == 1) {
      ASSERT_EQ(v->data, one.data);
    } else if (k == 2) {
      ASSERT_EQ(v->data, two.data);
    } else if (k == 3) {
      ASSERT_EQ(v->data, three.data);
    }
  }
}

UTEST(hash_map, drop_arrays) {
  HashMap<Array<u64>> map;
  defer({
    for (auto [k, v] : map) {
      drop(v);
    }
    drop(&map);
  });

  for (u64 i = 1; i <= 100; i++) {
    Array<u64> arr;
    reserve(&arr, i);
    for (u64 j = 0; j < i; j++) {
      push(&arr, j);
    }
    map[i] = arr;
  }
}

// main

UTEST_STATE();

static void dump_allocs(Allocator *a) {
  i32 allocs = 0;
  for (DebugAllocInfo *info = a->head; info != nullptr; info = info->next) {
    allocs++;
  }

  printf("  --- allocations (%d) ---\n", allocs);
  for (DebugAllocInfo *info = a->head; info != nullptr; info = info->next) {
    printf("  %10llu bytes: %s:%d\n", info->size, info->file, info->line);
  }
}

int main(int argc, char **argv) {
  int exit_code = utest_main(argc, argv);
  dump_allocs(&g_allocator);
  return exit_code;
}
