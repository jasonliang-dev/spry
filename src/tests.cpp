#include "arena.cpp"
#include "array.h"
#include "deps/utest.h"
#include "hash_map.h"
#include "priority_queue.h"
#include "strings.cpp"

// prelude

UTEST(prelude, string_null_term) {
  String str = "Hello World!";
  ASSERT_EQ(str.data[str.len], 0);
}

UTEST(prelude, string_lines) {
  String str = "line1\nl2\nthis is a very long line 3";
  SplitLinesIterator it = begin(SplitLines(str));

  ASSERT_EQ(*it, "line1");
  ++it;
  ASSERT_EQ(*it, "l2");
  ++it;
  ASSERT_EQ(*it, "this is a very long line 3");
  ++it;
}

UTEST(prelude, split_lines_non_null_term) {
  String str = "line1\nl2\nline 3 some extra stuff that won't be in the string";
  str.len = 15;

  SplitLinesIterator it = begin(SplitLines(str));

  ASSERT_EQ(*it, "line1");
  ++it;
  ASSERT_EQ(*it, "l2");
  ++it;
  ASSERT_EQ(*it, "line 3");
  ++it;
  ASSERT_EQ(*it, "");
  ++it;
}

// array

UTEST(array, empty) {
  Array<int> arr = {};
  array_trash(&arr);
}

UTEST(array, push) {
  Array<int> arr = {};
  defer(array_trash(&arr));

  array_push(&arr, 1);
  array_push(&arr, 2);
  array_push(&arr, 3);

  ASSERT_EQ(arr.len, 3);

  ASSERT_EQ(arr[0], 1);
  ASSERT_EQ(arr[1], 2);
  ASSERT_EQ(arr[2], 3);
}

UTEST(array, pop) {
  Array<int> arr = {};
  defer(array_trash(&arr));

  array_push(&arr, 1);
  array_push(&arr, 2);

  ASSERT_EQ(arr.len, 2);

  ASSERT_EQ(arr[0], 1);
  ASSERT_EQ(arr[1], 2);
}

UTEST(array, begin_end) {
  Array<int> arr = {};
  defer(array_trash(&arr));

  for (i32 i = 0; i < 1000; i++) {
    array_push(&arr, i);
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
  hashmap_trash(&map);
}

UTEST(hash_map, insert) {
  HashMap<String> map;
  defer(hashmap_trash(&map));

  map[10] = "ten";
  map[20] = "twenty";
  map[30] = "thirty";
}

UTEST(hash_map, lookup) {
  HashMap<String> map;
  defer(hashmap_trash(&map));

  map[10] = "ten";
  map[20] = "twenty";
  map[30] = "thirty";

  String *ten = hashmap_get(&map, 10);
  ASSERT_NE(ten, nullptr);
  ASSERT_STREQ("ten", ten->data);

  String *twenty = hashmap_get(&map, 20);
  ASSERT_NE(twenty, nullptr);
  ASSERT_STREQ("twenty", twenty->data);

  String *thirty = hashmap_get(&map, 30);
  ASSERT_NE(thirty, nullptr);
  ASSERT_STREQ("thirty", thirty->data);
}

UTEST(hash_map, unset) {
  HashMap<String> map;
  defer(hashmap_trash(&map));

  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  hashmap_unset(&map, 2);

  ASSERT_NE(hashmap_get(&map, 1), nullptr);
  ASSERT_EQ(hashmap_get(&map, 2), nullptr);
  ASSERT_NE(hashmap_get(&map, 3), nullptr);
}

UTEST(hash_map, iterator) {
  HashMap<String> map;
  defer(hashmap_trash(&map));

  String one = "one";
  String two = "two";
  String three = "three";
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
      array_trash(v);
    }
    hashmap_trash(&map);
  });

  for (u64 i = 1; i <= 100; i++) {
    Array<u64> arr;
    array_reserve(&arr, i);
    for (u64 j = 0; j < i; j++) {
      array_push(&arr, j);
    }
    map[i] = arr;
  }
}

UTEST(priority_queue, empty) {
  PriorityQueue<String> pq;
  defer(priority_queue_trash(&pq));
}

UTEST(priority_queue, push_pop) {
  PriorityQueue<String> pq;
  defer(priority_queue_trash(&pq));

  priority_queue_push(&pq, String("three"), 3);
  priority_queue_push(&pq, String("one"), 1);
  priority_queue_push(&pq, String("two"), 2);

  String s = {};

  ASSERT_TRUE(priority_queue_pop(&pq, &s));
  ASSERT_STREQ("one", s.data);

  ASSERT_TRUE(priority_queue_pop(&pq, &s));
  ASSERT_STREQ("two", s.data);

  ASSERT_TRUE(priority_queue_pop(&pq, &s));
  ASSERT_STREQ("three", s.data);
}

UTEST(priority_queue, same_costs) {
  PriorityQueue<String> pq;
  defer(priority_queue_trash(&pq));

  priority_queue_push(&pq, String("three"), 3);
  priority_queue_push(&pq, String("three"), 3);
  priority_queue_push(&pq, String("three"), 3);
  priority_queue_push(&pq, String("one"), 1);

  String s = {};

  ASSERT_TRUE(priority_queue_pop(&pq, &s));
  ASSERT_STREQ("one", s.data);

  for (i32 i = 0; i < 3; i++) {
    ASSERT_TRUE(priority_queue_pop(&pq, &s));
    ASSERT_STREQ("three", s.data);
  }
}

UTEST(arena, rebump) {
  Arena a = {};
  defer(arena_trash(&a));

  void *ptr0 = arena_bump(&a, 32);
  void *ptr1 = arena_rebump(&a, ptr0, 32, 64);
  ASSERT_EQ(ptr0, ptr1);
}

UTEST(arena, rebump_exhaust) {
  Arena a = {};
  defer(arena_trash(&a));

  void *ptr0 = arena_bump(&a, 32);
  arena_bump(&a, 4096);
  void *ptr1 = arena_rebump(&a, ptr0, 32, 64);
  ASSERT_NE(ptr0, ptr1);
}

UTEST(arena, rebump_exhaust_big) {
  Arena a = {};
  defer(arena_trash(&a));

  void *ptr0 = arena_bump(&a, 4000);
  arena_bump(&a, 32);
  void *ptr1 = arena_rebump(&a, ptr0, 4000, 8000);
  ASSERT_NE(ptr0, ptr1);
}

// main

UTEST_STATE();

/* extern(prelude.h) */ Allocator *g_allocator;

int main(int argc, char **argv) {
  g_allocator = new DebugAllocator();
  int exit_code = utest_main(argc, argv);

  DebugAllocator *allocator = (DebugAllocator *)g_allocator;
  i32 allocs = 0;
  for (DebugAllocInfo *info = allocator->head; info != nullptr;
       info = info->next) {
    printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size,
           info->file, info->line);
    allocs++;
  }
  printf("  --- %d allocation(s) ---\n", allocs);

  return exit_code;
}
