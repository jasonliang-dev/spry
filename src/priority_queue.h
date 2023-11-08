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
void priority_queue_swap(PriorityQueue<T> *q, i32 i, i32 j) {
  T t = q->data[i];
  q->data[i] = q->data[j];
  q->data[j] = t;

  float f = q->costs[i];
  q->costs[i] = q->costs[j];
  q->costs[j] = f;
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
void priority_queue_shift_up(PriorityQueue<T> *pq, i32 j) {
  while (j > 0) {
    i32 i = (j - 1) / 2;
    if (i == j || pq->costs[j] >= pq->costs[i]) {
      break;
    }
    priority_queue_swap(pq, i, j);
    j = i;
  }
}

template <typename T>
void priority_queue_shift_down(PriorityQueue<T> *pq, i32 i, i32 n) {
  if (i < 0 || i > n) {
    return;
  }

  while (true) {
    i32 j1 = 2 * i + 1;
    if (j1 < 0 || j1 >= n) {
      break;
    }

    i32 j = j1;
    i32 j2 = j1 + 1;
    if (j2 < n && pq->costs[j] < pq->costs[i]) {
      j = j2;
    }

    if (pq->costs[j] >= pq->costs[i]) {
      break;
    }

    priority_queue_swap(pq, i, j);
    i = j;
  }
}

template <typename T>
void priority_queue_push(PriorityQueue<T> *pq, T item, float cost) {
  if (pq->len == pq->capacity) {
    priority_queue_reserve(pq, pq->len * 2 + 1);
  }

  pq->data[pq->len] = item;
  pq->costs[pq->len] = cost;

  priority_queue_shift_up(pq, pq->len);
  pq->len++;
}

template <typename T>
bool priority_queue_pop(PriorityQueue<T> *pq, T *data, float *cost) {
  if (pq->len == 0) {
    return false;
  }

  *data = pq->data[0];
  *cost = pq->costs[0];

  pq->data[0] = pq->data[pq->len - 1];
  pq->costs[0] = pq->costs[pq->len - 1];
  pq->len--;

  priority_queue_shift_down(pq, 0, pq->len);

  return true;
}
