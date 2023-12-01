#pragma once

#include "prelude.h"

enum HashMapKind : u8 {
  HashMapKind_None,
  HashMapKind_Some,
  HashMapKind_Tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

template <typename T> struct HashMap {
  u64 *keys = nullptr;
  T *values = nullptr;
  HashMapKind *kinds = nullptr;
  u64 load = 0;
  u64 capacity = 0;
  T &operator[](u64 key);
};

template <typename T> void hashmap_trash(HashMap<T> *map) {
  mem_free(map->keys);
  mem_free(map->values);
  mem_free(map->kinds);
}

template <typename T> u64 hashmap_find_entry(const HashMap<T> *map, u64 key) {
  u64 index = key & (map->capacity - 1);
  u64 tombstone = (u64)-1;
  while (true) {
    HashMapKind kind = map->kinds[index];
    if (kind == HashMapKind_None) {
      return tombstone != (u64)-1 ? tombstone : index;
    } else if (kind == HashMapKind_Tombstone) {
      tombstone = index;
    } else if (map->keys[index] == key) {
      return index;
    }

    index = (index + 1) & (map->capacity - 1);
  }
}

template <typename T> void hashmap_real_reserve(HashMap<T> *old, u64 capacity) {
  if (capacity <= old->capacity) {
    return;
  }

  HashMap<T> map = {};
  map.capacity = capacity;

  size_t bytes = sizeof(u64) * capacity;
  map.keys = (u64 *)mem_alloc(bytes);
  memset(map.keys, 0, bytes);

  map.values = (T *)mem_alloc(sizeof(T) * capacity);
  memset(map.values, 0, sizeof(T) * capacity);

  map.kinds = (HashMapKind *)mem_alloc(sizeof(HashMapKind) * capacity);
  memset(map.kinds, 0, sizeof(HashMapKind) * capacity);

  for (u64 i = 0; i < old->capacity; i++) {
    HashMapKind kind = old->kinds[i];
    if (kind != HashMapKind_Some) {
      continue;
    }

    u64 index = hashmap_find_entry(&map, old->keys[i]);
    map.keys[index] = old->keys[i];
    map.values[index] = old->values[i];
    map.kinds[index] = HashMapKind_Some;
    map.load++;
  }

  mem_free(old->keys);
  mem_free(old->values);
  mem_free(old->kinds);
  *old = map;
}

inline u64 hash_map_reserve_size(u64 size) {
  u64 n = (u64)(size / HASH_MAP_LOAD_FACTOR) + 1;

  // next pow of 2
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;

  return n;
}

template <typename T> void hashmap_reserve(HashMap<T> *old, u64 capacity) {
  capacity = hash_map_reserve_size(capacity);
  hashmap_real_reserve(old, capacity);
}

template <typename T> T *hashmap_get(HashMap<T> *map, u64 key) {
  if (map->load == 0) {
    return nullptr;
  }

  u64 index = hashmap_find_entry(map, key);
  return map->kinds[index] == HashMapKind_Some ? &map->values[index] : nullptr;
}

template <typename T> const T *hashmap_get(const HashMap<T> *map, u64 key) {
  if (map->load == 0) {
    return nullptr;
  }

  u64 index = hashmap_find_entry(map, key);
  return map->kinds[index] == HashMapKind_Some ? &map->values[index] : nullptr;
}

template <typename T> bool hashmap_index(HashMap<T> *map, u64 key, T **value) {
  if (map->load >= map->capacity * HASH_MAP_LOAD_FACTOR) {
    hashmap_real_reserve(map, map->capacity > 0 ? map->capacity * 2 : 16);
  }

  u64 index = hashmap_find_entry(map, key);
  bool exists = map->kinds[index] == HashMapKind_Some;
  if (!exists) {
    map->values[index] = {};
  }

  if (map->kinds[index] == HashMapKind_None) {
    map->load++;
    map->keys[index] = key;
    map->kinds[index] = HashMapKind_Some;
  }

  *value = &map->values[index];
  return exists;
}

template <typename T> T &HashMap<T>::operator[](u64 key) {
  T *value;
  hashmap_index(this, key, &value);
  return *value;
}

template <typename T> void hashmap_unset(HashMap<T> *map, u64 key) {
  if (map->load == 0) {
    return;
  }

  u64 index = hashmap_find_entry(map, key);
  if (map->kinds[index] != HashMapKind_None) {
    map->kinds[index] = HashMapKind_Tombstone;
  }
}

template <typename T> void hashmap_clear(HashMap<T> *map) {
  memset(map->keys, 0, sizeof(u64) * map->capacity);
  memset(map->values, 0, sizeof(T) * map->capacity);
  memset(map->kinds, 0, sizeof(HashMapKind) * map->capacity);
  map->load = 0;
}

template <typename T> struct HashMapKV {
  u64 key;
  T *value;
};

template <typename T> struct HashMapIterator {
  HashMap<T> *map;
  u64 cursor;

  HashMapKV<T> operator*() const {
    HashMapKV<T> kv;
    kv.key = map->keys[cursor];
    kv.value = &map->values[cursor];
    return kv;
  }

  HashMapIterator &operator++() {
    cursor++;
    while (cursor != map->capacity) {
      if (map->kinds[cursor] == HashMapKind_Some) {
        return *this;
      }
      cursor++;
    }

    return *this;
  }
};

template <typename T>
bool operator!=(HashMapIterator<T> lhs, HashMapIterator<T> rhs) {
  return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T> HashMapIterator<T> begin(HashMap<T> &map) {
  HashMapIterator<T> it = {};
  it.map = &map;
  it.cursor = map.capacity;

  for (u64 i = 0; i < map.capacity; i++) {
    if (map.kinds[i] == HashMapKind_Some) {
      it.cursor = i;
      break;
    }
  }

  return it;
}

template <typename T> HashMapIterator<T> end(HashMap<T> &map) {
  HashMapIterator<T> it = {};
  it.map = &map;
  it.cursor = map.capacity;
  return it;
}
