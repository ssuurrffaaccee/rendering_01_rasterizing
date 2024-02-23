#pragma once
#include <algorithm>

#include "vec.hpp"
template <typename Type, size_t Len>
using Square = std::array<Type, Len * Len>;

template <typename Type, size_t Len>
Square<Type, Len> mmdot(const Square<Type, Len> &r, const Square<Type, Len> &l) {
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

using Square3 = Square<float, 3>;
using Square4 = Square<float, 4>;
