#pragma once

#include "prelude.h"

template <typename T> struct PriorityQueue {
  T *data = nullptr;
  float *costs = nullptr;
  u64 len = 0;
  u64 capacity = 0;
};

template <typename T> void priority_queue_trash(PriorityQueue<T> *pq) {
  mem_free(pq->data);
  mem_free(pq->costs);
}

template <typename T>
void priority_queue_reserve(PriorityQueue<T> *pq, u64 capacity) {
  if (capacity <= pq->capacity) {
    return;
  }

  T *data = (T *)mem_alloc(sizeof(T) * capacity);
  memcpy(data, pq->data, sizeof(T) * pq->len);
  mem_free(pq->data);
  pq->data = data;

  float *costs = (float *)mem_alloc(sizeof(float) * capacity);
  memcpy(costs, pq->costs, sizeof(float) * pq->len);
  mem_free(pq->costs);
  pq->costs = costs;

  pq->capacity = capacity;
}

template <typename T>
void priority_queue_swap(PriorityQueue<T> *pq, i32 i, i32 j) {
  T t = pq->data[i];
  pq->data[i] = pq->data[j];
  pq->data[j] = t;

  float f = pq->costs[i];
  pq->costs[i] = pq->costs[j];
  pq->costs[j] = f;
}

template <typename T>
void priority_queue_push(PriorityQueue<T> *pq, T item, float cost) {
  if (pq->len == pq->capacity) {
    priority_queue_reserve(pq, pq->len * 2 + 1);
  }

  pq->data[pq->len] = item;
  pq->costs[pq->len] = cost;
  pq->len++;

  for (i32 i = pq->len; i > 1 && pq->costs[i - 1] < pq->costs[i / 2 - 1];
       i /= 2) {
    priority_queue_swap(pq, i - 1, i / 2 - 1);
  }
}

template <typename T> bool priority_queue_pop(PriorityQueue<T> *pq, T *data) {
  if (pq->len == 0) {
    return false;
  }

  *data = pq->data[0];

  pq->data[0] = pq->data[pq->len - 1];
  pq->costs[0] = pq->costs[pq->len - 1];
  pq->len--;

  i32 i = 0, j = 1;
  while (i != j) {
    i = j;
    if (2 * i + 1 <= pq->len) {
      if (pq->costs[i - 1] > pq->costs[2 * i - 1]) {
        j = 2 * i;
      }
      if (pq->costs[j - 1] > pq->costs[2 * i + 1 - 1]) {
        j = 2 * i + 1;
      }

    } else if (2 * i <= pq->len) {
      if (pq->costs[i - 1] > pq->costs[2 * i - 1]) {
        j = 2 * i;
      }
    }

    if (i != j) {
      priority_queue_swap(pq, i - 1, j - 1);
    }
  }

  return true;
}
