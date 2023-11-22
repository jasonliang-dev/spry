#pragma once

#include "prelude.h"

template <typename T> struct Chan {
  Mutex mtx = {};
  Cond cv = {};

  T *data = nullptr;
  u64 front = 0;
  u64 back = 0;
  u64 len = 0;
  u64 capacity = 0;
};

template <typename T> void chan_make(Chan<T> *c) {
  c->mtx = mutex_make();
  c->cv = cond_make();
}

template <typename T> void chan_trash(Chan<T> *c) {
  mutex_trash(&c->mtx);
  cond_trash(&c->cv);
  mem_free(c->data);
}

template <typename T> void chan_reserve(Chan<T> *c, u64 capacity) {
  if (capacity > c->capacity) {
    T *buf = (T *)mem_alloc(sizeof(T) * capacity);

    u64 lhs = c->back;
    u64 rhs = (c->capacity - c->front);

    memcpy(buf, &c->data[c->front], sizeof(T) * rhs);
    memcpy(&buf[rhs], &c->data[0], sizeof(T) * lhs);

    mem_free(c->data);

    c->data = buf;
    c->front = 0;
    c->back = c->len;
    c->capacity = capacity;
  }
}

template <typename T> void chan_send(Chan<T> *c, T item) {
  mutex_lock(&c->mtx);
  defer(mutex_unlock(&c->mtx));

  if (c->len == c->capacity) {
    i32 grow = c->len > 0 ? c->len * 2 : 8;
    chan_reserve(c, grow);
  }

  c->data[c->back] = item;
  c->back = (c->back + 1) % c->capacity;
  c->len++;

  cond_signal(&c->cv);
}

template <typename T> T chan_recv(Chan<T> *c) {
  mutex_lock(&c->mtx);
  defer(mutex_unlock(&c->mtx));

  while (c->len == 0) {
    cond_wait(&c->cv, &c->mtx);
  }

  T data = c->data[c->front];
  c->front = (c->front + 1) % c->capacity;
  c->len--;

  return data;
}
