#pragma once

#include "prelude.h"

template <typename T> struct PriorityQueue {
  T *data = nullptr;
  u64 len = 0;
  u64 capacity = 0;
  bool (*cmp)(T lhs, T rhs);
};

template <typename T> void priority_queue_trash(PriorityQueue<T> *pq) {
  mem_free(pq->data);
}

template <typename T>
void priority_queue_swap(PriorityQueue<T> *q, i32 i, i32 j) {
  T t = q->data[i];
  q->data[i] = q->data[j];
  q->data[j] = t;
}

template <typename T>
void priority_queue_reserve(PriorityQueue<T> *pq, u64 capacity) {
  if (capacity <= pq->capacity) {
    return;
  }

  T *buf = (T *)mem_alloc(sizeof(T) * capacity);
  memcpy(buf, pq->data, sizeof(T) * pq->len);
  mem_free(pq->data);
  pq->data = buf;

  pq->capacity = capacity;
}

template <typename T>
void priority_queue_shift_up(PriorityQueue<T> *pq, i32 j) {
  while (j > 0) {
    i32 i = (j - 1) / 2;
    if (i == j || !pq->cmp(pq->data[j], pq->data[i])) {
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
    if (j2 < n && pq->cmp(pq->data[j], pq->data[i])) {
      j = j2;
    }

    if (!pq->cmp(pq->data[j], pq->data[i])) {
      break;
    }

    priority_queue_swap(pq, i, j);
    i = j;
  }
}

template <typename T>
void priority_queue_push(PriorityQueue<T> *pq, T item) {
  if (pq->len == pq->capacity) {
    priority_queue_reserve(pq, pq->len * 2 + 1);
  }

  pq->data[pq->len] = item;

  priority_queue_shift_up(pq, pq->len);
  pq->len++;
}

template <typename T> bool priority_queue_pop(PriorityQueue<T> *pq, T *out) {
  if (pq->len == 0) {
    return false;
  }

  *out = pq->data[0];

  pq->data[0] = pq->data[pq->len - 1];
  pq->len--;

  priority_queue_shift_down(pq, 0, pq->len);

  return true;
}
