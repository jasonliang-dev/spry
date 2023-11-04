#pragma once

#if defined(_M_AMD64) || defined(__SSE__)
#ifndef SSE_AVAILABLE
#define SSE_AVAILABLE
#endif
#include <immintrin.h>
#endif

union Vector4 {
  struct {
    float x, y, z, w;
  };
#ifdef SSE_AVAILABLE
  __m128 sse;
#endif
};

inline Vector4 vec4(float x, float y, float z, float w) {
  Vector4 v = {};
#ifdef SSE_AVAILABLE
  v.sse = _mm_set_ps(w, z, y, x);
#else
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = w;
#endif
  return v;
}

inline Vector4 vec4_xy(float x, float y) {
  Vector4 v = {};
#ifdef SSE_AVAILABLE
  v.sse = _mm_set_ps(1, 0, y, x);
#else
  v.x = x;
  v.y = y;
  v.w = 1;
#endif
  return v;
}

union Matrix4 {
  float cols[4][4];
  Vector4 vecs[4];
#ifdef SSE_AVAILABLE
  __m128 sse[4];
#endif
};

inline Vector4 vec4_mul_mat4(Vector4 v, Matrix4 m) {
#ifdef SSE_AVAILABLE
  __m128 mul0 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0x00), m.sse[0]);
  __m128 mul1 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0x55), m.sse[1]);
  __m128 mul2 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0xaa), m.sse[2]);
  __m128 mul3 = _mm_mul_ps(_mm_shuffle_ps(v.sse, v.sse, 0xff), m.sse[3]);

  Vector4 out = {};
  out.sse = _mm_add_ps(_mm_add_ps(mul0, mul1), _mm_add_ps(mul2, mul3));
  return out;
#else
  Vector4 out = {};
  out.x = v.x * m.cols[0][0];
  out.y = v.x * m.cols[0][1];
  out.z = v.x * m.cols[0][2];
  out.w = v.x * m.cols[0][3];

  out.x += v.y * m.cols[1][0];
  out.y += v.y * m.cols[1][1];
  out.z += v.y * m.cols[1][2];
  out.w += v.y * m.cols[1][3];

  out.x += v.z * m.cols[2][0];
  out.y += v.z * m.cols[2][1];
  out.z += v.z * m.cols[2][2];
  out.w += v.z * m.cols[2][3];

  out.x += v.w * m.cols[3][0];
  out.y += v.w * m.cols[3][1];
  out.z += v.w * m.cols[3][2];
  out.w += v.w * m.cols[3][3];
  return out;
#endif
}

inline Matrix4 mat4_mul_mat4(Matrix4 lhs, Matrix4 rhs) {
  Matrix4 m = {};
  m.vecs[0] = vec4_mul_mat4(rhs.vecs[0], lhs);
  m.vecs[1] = vec4_mul_mat4(rhs.vecs[1], lhs);
  m.vecs[2] = vec4_mul_mat4(rhs.vecs[2], lhs);
  m.vecs[3] = vec4_mul_mat4(rhs.vecs[3], lhs);
  return m;
}
