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

  void trash() {
    mem_free(keys);
    mem_free(values);
    mem_free(kinds);
  }

  u64 find_entry(u64 key) const {
    u64 index = key & (capacity - 1);
    u64 tombstone = (u64)-1;
    while (true) {
      HashMapKind kind = kinds[index];
      if (kind == HashMapKind_None) {
        return tombstone != (u64)-1 ? tombstone : index;
      } else if (kind == HashMapKind_Tombstone) {
        tombstone = index;
      } else if (keys[index] == key) {
        return index;
      }

      index = (index + 1) & (capacity - 1);
    }
  }

  void real_reserve(u64 cap) {
    if (cap <= capacity) {
      return;
    }

    HashMap<T> map = {};
    map.capacity = cap;

    size_t bytes = sizeof(u64) * cap;
    map.keys = (u64 *)mem_alloc(bytes);
    memset(map.keys, 0, bytes);

    map.values = (T *)mem_alloc(sizeof(T) * cap);
    memset(map.values, 0, sizeof(T) * cap);

    map.kinds = (HashMapKind *)mem_alloc(sizeof(HashMapKind) * cap);
    memset(map.kinds, 0, sizeof(HashMapKind) * cap);

    for (u64 i = 0; i < capacity; i++) {
      HashMapKind kind = kinds[i];
      if (kind != HashMapKind_Some) {
        continue;
      }

      u64 index = map.find_entry(keys[i]);
      map.keys[index] = keys[i];
      map.values[index] = values[i];
      map.kinds[index] = HashMapKind_Some;
      map.load++;
    }

    mem_free(keys);
    mem_free(values);
    mem_free(kinds);
    *this = map;
  }

  void reserve(u64 capacity) {
    u64 n = (u64)(capacity / HASH_MAP_LOAD_FACTOR) + 1;

    // next pow of 2
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    real_reserve(n);
  }

  T *get(u64 key) {
    if (load == 0) {
      return nullptr;
    }
    u64 index = find_entry(key);
    return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
  }

  const T *get(u64 key) const {
    if (load == 0) {
      return nullptr;
    }
    u64 index = find_entry(key);
    return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
  }

  bool find_or_insert(u64 key, T **value) {
    if (load >= capacity * HASH_MAP_LOAD_FACTOR) {
      real_reserve(capacity > 0 ? capacity * 2 : 16);
    }

    u64 index = find_entry(key);
    bool exists = kinds[index] == HashMapKind_Some;
    if (!exists) {
      values[index] = {};
    }

    if (kinds[index] == HashMapKind_None) {
      load++;
      keys[index] = key;
      kinds[index] = HashMapKind_Some;
    }

    *value = &values[index];
    return exists;
  }

  T &operator[](u64 key) {
    T *value;
    find_or_insert(key, &value);
    return *value;
  }

  void unset(u64 key) {
    if (load == 0) {
      return;
    }

    u64 index = find_entry(key);
    if (kinds[index] != HashMapKind_None) {
      kinds[index] = HashMapKind_Tombstone;
    }
  }

  void clear() {
    memset(keys, 0, sizeof(u64) * capacity);
    memset(values, 0, sizeof(T) * capacity);
    memset(kinds, 0, sizeof(HashMapKind) * capacity);
    load = 0;
  }
};

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
