#pragma once
#include <array>
#include <cmath>
#include <sstream>
#include <vector>

#include "random.hpp"
template <typename Type, size_t Len>
using Vec = std::array<Type, Len>;

#define BINARY_OPERATION_FOR_VEC(OP)                    \
  template <typename Type, size_t Len>                  \
  Vec<Type, Len> operator OP(const Vec<Type, Len> &r,   \
                             const Vec<Type, Len> &l) { \
    Vec<Type, Len> res;                                 \
    for (size_t i = 0u; i < Len; i++) {                 \
      res[i] = r[i] OP l[i];                            \
    }                                                   \
    return res;                                         \
  }
BINARY_OPERATION_FOR_VEC(+)
BINARY_OPERATION_FOR_VEC(-)
BINARY_OPERATION_FOR_VEC(*)
BINARY_OPERATION_FOR_VEC(/)

#define BINARY_ASSIGN_OPERATION_FOR_VEC(OP)                      \
  template <typename Type, size_t Len>                           \
  void operator OP(Vec<Type, Len> &r, const Vec<Type, Len> &l) { \
    for (size_t i = 0u; i < Len; i++) {                          \
      r[i] OP l[i];                                              \
    }                                                            \
  }
BINARY_ASSIGN_OPERATION_FOR_VEC(+=)
BINARY_ASSIGN_OPERATION_FOR_VEC(-=)
BINARY_ASSIGN_OPERATION_FOR_VEC(*=)
BINARY_ASSIGN_OPERATION_FOR_VEC(/=)

#define BINARY_ASSIGN_OPERATION_FOR_VEC_WITH_SCALAR(OP)      \
  template <typename Type, size_t Len, typename ScalarType>  \
  void operator OP(Vec<Type, Len> &r, const ScalarType &l) { \
    for (size_t i = 0u; i < Len; i++) {                      \
      r[i] OP l;                                             \
    }                                                        \
  }
BINARY_ASSIGN_OPERATION_FOR_VEC_WITH_SCALAR(+=)
BINARY_ASSIGN_OPERATION_FOR_VEC_WITH_SCALAR(-=)
BINARY_ASSIGN_OPERATION_FOR_VEC_WITH_SCALAR(*=)
BINARY_ASSIGN_OPERATION_FOR_VEC_WITH_SCALAR(/=)

#define BINARY_OPERATION_FOR_VEC_WITH_SCALAR_1(OP)                            \
  template <typename Type, size_t Len, typename ScalarType>                   \
  Vec<Type, Len> operator OP(const Vec<Type, Len> &r, const ScalarType & l) { \
    Vec<Type, Len> res;                                                       \
    for (size_t i = 0u; i < Len; i++) {                                       \
      res[i] = r[i] OP l;                                                     \
    }                                                                         \
    return res;                                                               \
  }
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_1(+)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_1(-)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_1(*)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_1(/)

#define BINARY_OPERATION_FOR_VEC_WITH_SCALAR_2(OP)                            \
  template <typename Type, size_t Len, typename ScalarType>                   \
  Vec<Type, Len> operator OP(const ScalarType & l, const Vec<Type, Len> &r) { \
    Vec<Type, Len> res;                                                       \
    for (size_t i = 0u; i < Len; i++) {                                       \
      res[i] = l OP r[i];                                                     \
    }                                                                         \
    return res;                                                               \
  }
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_2(+)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_2(-)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_2(*)
BINARY_OPERATION_FOR_VEC_WITH_SCALAR_2(/)

// BINARY_ASSIGN_OPERATION_FOR_VEC(+=)
// BINARY_ASSIGN_OPERATION_FOR_VEC(-=)
// BINARY_ASSIGN_OPERATION_FOR_VEC(*=)
// BINARY_ASSIGN_OPERATION_FOR_VEC(/=)

#define MAP(FUNC)                                \
  template <typename Type, size_t Len>           \
  Vec<Type, Len> FUNC(const Vec<Type, Len> &r) { \
    Vec<Type, Len> res;                          \
    for (size_t i = 0u; i < Len; i++) {          \
      res[i] = std::FUNC(r[i]);                  \
    }                                            \
    return res;                                  \
  }
MAP(sin)
MAP(cos)
MAP(ceil)
MAP(floor)
MAP(abs)

template <typename Type, size_t Len>
float length(const Vec<Type, Len> &vs) {
  float sum = 0;
  for (size_t i = 0u; i < Len; i++) {
    sum += vs[i] * vs[i];
  }
  return std::sqrt(sum);
}

template <typename Type, size_t Len>
Vec<Type, Len> normalize(const Vec<Type, Len> &vs) {
  Vec<Type, Len> res;
  auto norm = length(vs);
  for (size_t i = 0u; i < Len; i++) {
    res[i] = vs[i] / norm;
  }
  return res;
}

template <typename Type, size_t Len>
float dot(const Vec<Type, Len> &r, const Vec<Type, Len> &l) {
  float res{0.0f};
  for (size_t i = 0u; i < Len; i++) {
    res += r[i] * l[i];
  }
  return res;
}

template <typename Type, size_t Len>
float min(const Vec<Type, Len> &r) {
  float min_data = r[0];
  for (size_t i = 1u; i < Len; i++) {
    if (min_data > r[i]) {
      min_data = r[i];
    }
  }
  return min_data;
}
template <typename Type, size_t Len>
float max(const Vec<Type, Len> &r) {
  float max_data = r[0];
  for (size_t i = 1u; i < Len; i++) {
    if (max_data < r[i]) {
      max_data = r[i];
    }
  }
  return max_data;
}

template <typename Type, size_t Len>
Vec<Type, Len> clamp(const Vec<Type, Len> &vs, float low, float high) {
  Vec<Type, Len> res;
  for (size_t i = 0u; i < Len; i++) {
    if (vs[i] < low) {
      res[i] = low;
      continue;
    }
    if (vs[i] > high) {
      res[i] = high;
      continue;
    }
    res[i] = vs[i];
  }
  return res;
}

float clamp(float vs, float low, float high) {
  if (vs < low) {
    return low;
  }
  if (vs > high) {
    return high;
  }
  return vs;
}

template <typename Type, size_t Len>
Vec<Type, Len> avg(const std::vector<Vec<Type, Len>> &vs) {
  Vec<Type, Len> res{0.0f, 0.0f, 0.0f};
  for (size_t i = 0u; i < vs.size(); i++) {
    res += vs[i];
  }
  return res / float(vs.size());
}

float mix(float l, float r, float ratio) { return l * ratio + (1 - ratio) * r; }
int mix(int l, int r, float ratio) {
  return int(float(l) * ratio + (1 - ratio) * float(r));
}
template <typename Type, size_t Len>
Vec<Type, Len> mix(const Vec<Type, Len> &r, const Vec<Type, Len> &l,
                   float ratio) {
  Vec<Type, Len> res;
  for (size_t i = 0u; i < Len; i++) {
    res[i] = ::mix(r[i], l[i], ratio);
  }
  return res;
}
template <typename Type, size_t Len>
Vec<Type, Len> mod(const Vec<Type, Len> &r, const float &l) {
  Vec<Type, Len> res;
  for (size_t i = 0u; i < Len; i++) {
    res[i] = std::fmod(r[i], l);
  }
  return res;
}
template <typename Type, size_t Len>
Vec<Type, Len + 1> extend(const Vec<Type, Len> &v, float s) {
  Vec<Type, Len + 1> res;
  for (size_t i = 0u; i < Len; i++) {
    res[i] = v[i];
  }
  res[Len] = s;
  return res;
}
template <typename Type, size_t Len>
Vec<Type, Len - 1> shrink(const Vec<Type, Len> &v) {
  Vec<Type, Len - 1> res;
  for (size_t i = 0u; i < Len - 1; i++) {
    res[i] = v[i];
  }
  return res;
}

template <typename Type, size_t Len>
std::string to_string(const Vec<Type, Len> &vec) {
  std::stringstream ss;
  ss << "vec(";
  ss << vec[0];
  for (size_t i = 1u; i < Len; i++) {
    ss << ", " << vec[i];
  }
  ss << ")";
  return ss.str();
}

std::string to_string(std::vector<float> &vs) {
  std::stringstream ss;
  ss << "array(";
  for (auto &v : vs) {
    ss << v << ",";
  }
  ss << ")";
  return ss.str();
}

using Vec2 = Vec<float, 2>;
using Vec3 = Vec<float, 3>;
using Vec4 = Vec<float, 4>;
using Color = Vec3;

Color gray(float density) { return Color{density, density, density}; }

Vec3 cross(const Vec3 &r, const Vec3 &l) {
  Vec3 res;
  res[0] = r[1] * l[2] - r[2] * l[1];
  res[1] = r[2] * l[0] - r[0] * l[2];
  res[2] = r[0] * l[1] - r[1] * l[0];
  return res;
}

template <size_t Dim1, size_t Dim2>
struct SpaceBase;
template <>
struct SpaceBase<2, 2> {
  Vec<float, 2> bx, by;
};
template <>
struct SpaceBase<2, 3> {
  Vec<float, 3> bx, by;
};
template <>
struct SpaceBase<3, 3> {
  Vec<float, 3> bx, by, bz;
};
using SpaceBase3D = SpaceBase<3, 3>;
using SpaceBase2D = SpaceBase<2, 2>;
using SpaceBase2DIn3D = SpaceBase<2, 3>;

Vec2 random_in_unit_square() { return Vec2{my_random(), my_random()}; }

/*
contract:
  +u = +x
  +v = +y
  +w = +z
       top
  left front right back
       bottom
*/
inline int arg_abs_max(float u, float v, float w) {
  u = std::abs(u);
  v = std::abs(v);
  w = std::abs(w);
  int index = 0;
  float max = u;
  if (max < v) {
    index = 1;
    max = v;
  }
  if (max < w) {
    index = 2;
    max = w;
  }
  return index;
}