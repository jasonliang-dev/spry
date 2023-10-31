#pragma once

#include "prelude.h"

struct Vector4 {
  float x, y, z, w;
};

union Matrix4 {
  float cols[4][4];
  Vector4 vecs[4];
};

inline Vector4 vec4_mul_mat4(Vector4 v, Matrix4 m) {
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
}

inline Matrix4 mat4_mul_mat4(Matrix4 lhs, Matrix4 rhs) {
  Matrix4 m = {};
  m.vecs[0] = vec4_mul_mat4(rhs.vecs[0], lhs);
  m.vecs[1] = vec4_mul_mat4(rhs.vecs[1], lhs);
  m.vecs[2] = vec4_mul_mat4(rhs.vecs[2], lhs);
  m.vecs[3] = vec4_mul_mat4(rhs.vecs[3], lhs);
  return m;
}
