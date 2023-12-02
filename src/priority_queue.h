#pragma once

#include "prelude.h"

template <typename T> struct PriorityQueue {
  T *data = nullptr;
  float *costs = nullptr;
  u64 len = 0;
  u64 capacity = 0;

  void trash() {
    mem_free(data);
    mem_free(costs);
  }

  void reserve(u64 cap) {
    if (cap <= capacity) {
      return;
    }

    T *buf = (T *)mem_alloc(sizeof(T) * cap);
    memcpy(buf, data, sizeof(T) * len);
    mem_free(data);
    data = buf;

    float *cbuf = (float *)mem_alloc(sizeof(float) * cap);
    memcpy(cbuf, costs, sizeof(float) * len);
    mem_free(costs);
    costs = cbuf;

    capacity = cap;
  }

  void swap(i32 i, i32 j) {
    T t = data[i];
    data[i] = data[j];
    data[j] = t;

    float f = costs[i];
    costs[i] = costs[j];
    costs[j] = f;
  }

  void shift_up(i32 j) {
    while (j > 0) {
      i32 i = (j - 1) / 2;
      if (i == j || costs[i] < costs[j]) {
        break;
      }

      swap(i, j);
      j = i;
    }
  }

  void shift_down(i32 i, i32 n) {
    if (i < 0 || i > n) {
      return;
    }

    i32 j = 2 * i + 1;
    while (j >= 0 && j < n) {
      if (j + 1 < n && costs[j + 1] < costs[j]) {
        j = j + 1;
      }

      if (costs[i] < costs[j]) {
        break;
      }

      swap(i, j);
      i = j;
      j = 2 * i + 1;
    }
  }

  void push(T item, float cost) {
    if (len == capacity) {
      reserve(len > 0 ? len * 2 : 8);
    }

    data[len] = item;
    costs[len] = cost;
    len++;

    shift_up(len - 1);
  }

  bool pop(T *item) {
    if (len == 0) {
      return false;
    }

    *item = data[0];

    data[0] = data[len - 1];
    costs[0] = costs[len - 1];
    len--;

    shift_down(0, len);
    return true;
  }
};
