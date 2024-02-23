#pragma once
#include <algorithm>

#include "vec.hpp"
template <typename Type, size_t Len>
using Square = std::array<Type, Len * Len>;

template <typename Type, size_t Len>
Square<Type, Len> mmdot(const Square<Type, Len> &r,
                        const Square<Type, Len> &l) {
  Square<Type, Len> res;
  res.fill(0.0f);
  for (size_t i = 0u; i < Len; i++) {
    for (size_t j = 0u; j < Len; j++) {
      for (size_t k = 0u; k < Len; k++) {
        *(res.begin() + i * Len + j) +=
            *(r.begin() + i * Len + k) * *(l.begin() + k * Len + j);
      }
    }
  }
  return res;
}

template <typename Type, size_t Len>
Vec<Type, Len> mvdot(const Square<Type, Len> &r, const Vec<Type, Len> &l) {
  Vec<Type, Len> res;
  res.fill(0.0f);
  for (size_t i = 0u; i < Len; i++) {
    for (size_t j = 0u; j < Len; j++) {
      res[i] += *(r.begin() + i * Len + j) * l[j];
    }
  }
  return res;
}
template <typename Type, size_t Len>
Vec<Type, Len * Len> eye() {
  Vec<Type, Len * Len> res;
  res.fill(0.0f);
  for (size_t i = 0u; i < Len; i++) {
    *(res.begin() + i * Len + i) = 1.0f;
  }
  return res;
}

template <typename Type, size_t Len>
Square<Type, Len> transpose(const Square<Type, Len> &m) {
  Square<Type, Len> res;
  for (size_t x = 0; x < Len; x++) {
    for (size_t y = 0; y < Len; y++) {
      res[x * Len + y] = m[y * Len + x];
    }
  }
  return res;
}

using Square2 = Square<float, 2>;
using Square3 = Square<float, 3>;
using Square4 = Square<float, 4>;

float determinant2(const Square2 &m2) { return m2[0] * m2[3] - m2[1] * m2[2]; }
Square2 adjugate2(const Square2 &m2) {
  return Square2{m2[3], -1.0f * m2[1], -1.0f * m2[2], m2[0]};
}
float _determinant2(float v0, float v1, float v2, float v3) {
  // std::cout<<v0<<" "<<v1<<" "<<v2<<" "<<v3<<"\n";
  return v0 * v3 - v1 * v2;
}
Square3 adjugate3(const Square3 &m3) {
#define REF(m, x, y, len) (m.begin() + x * len + y)
#define ONE(y, x, x0, x1, y0, y1)                                 \
  *REF(res, x, y, len) =                                          \
      (1 - ((x + y) % 2) * 2) *                                   \
      _determinant2(*REF(m3, x0, y0, len), *REF(m3, x0, y1, len), \
                    *REF(m3, x1, y0, len), *REF(m3, x1, y1, len));
  Square3 res{};
  int len = 3;
  ONE(0, 0, 1, 2, 1, 2)
  ONE(0, 1, 1, 2, 0, 2)
  ONE(0, 2, 1, 2, 0, 1)
  ONE(1, 0, 0, 2, 1, 2)
  ONE(1, 1, 0, 2, 0, 2)
  ONE(1, 2, 0, 2, 0, 1)
  ONE(2, 0, 0, 1, 1, 2)
  ONE(2, 1, 0, 1, 0, 2)
  ONE(2, 2, 0, 1, 0, 1)
#undef REF
#undef ONE
return res;
}
float determinant3(const Square3 &m3) {
#define REF(m, x, y, len) (m.begin() + x * len + y)
#define ONE(x, y, x0, x1, y0, y1)                                    \
  res += (1 - ((x + y) % 2) * 2) * *REF(m3, x, y, len) *             \
         _determinant2(*REF(m3, x0, y0, len), *REF(m3, x0, y1, len), \
                       *REF(m3, x1, y0, len), *REF(m3, x1, y1, len));
  float res{};
  int len = 3;
  ONE(0, 0, 1, 2, 1, 2)
  ONE(0, 1, 1, 2, 0, 2)
  ONE(0, 2, 1, 2, 0, 1)
  return res;
#undef REF
}
// float determinant4(const Square2 &m2) {}

Square2 inverse2(const Square2 &m2) { return adjugate2(m2) / determinant2(m2); }
Square3 inverse3(const Square3 &m3) { return adjugate3(m3) / determinant3(m3); }
// Square4 inverse4(const Square4 &m4) { return Square4{}; }

Square3 shrink(const Square4 &m4) {
  Square3 res{};
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      res[i * 3 + j] = m4[i * 4 + j];
    }
  }
  return res;
}
