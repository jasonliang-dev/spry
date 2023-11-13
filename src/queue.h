#pragma once

#include "prelude.h"

template <typename T> struct Queue {
  T *data = nullptr;
  u64 front = 0;
  u64 back = 0;
  u64 len = 0;
  u64 capacity = 0;
};

template <typename T> void queue_trash(Queue<T> *q) { mem_free(q->data); }

template <typename T> void queue_reserve(Queue<T> *q, u64 capacity) {
  if (capacity > q->capacity) {
    T *buf = (T *)mem_alloc(sizeof(T) * capacity);

    u64 lhs = q->back;
    u64 rhs = (q->capacity - q->front);

    memcpy(buf, &q->data[q->front], sizeof(T) * rhs);
    memcpy(&buf[rhs], &q->data[0], sizeof(T) * lhs);

    mem_free(q->data);

    q->data = buf;
    q->front = 0;
    q->back = q->len;
    q->capacity = capacity;
  }
}

template <typename T> void queue_push(Queue<T> *q, T item) {
  if (q->len == q->capacity) {
    i32 grow = q->len > 0 ? q->len * 2 : 8;
    queue_reserve(q, grow);
  }
  q->data[q->back] = item;

  q->len++;
  q->back = (q->back + 1) % q->capacity;
}

template <typename T> bool queue_pop(Queue<T> *q, T *data) {
  if (q->len == 0) {
    return false;
  }

  *data = q->data[q->front];

  q->len--;
  q->front = (q->front + 1) % q->capacity;
  return true;
}
