#pragma once
#include "vec.hpp"
#include "mat.hpp"
Square4 to_scale_transform(Vec3 scale) {
  Square4 transform = eye<float, 4>();
  // file scale
  for (size_t row = 0; row < 3; row++) {
    size_t col = row;
    *(transform.begin() + 4 * row + col) = *(scale.begin() + row);
  }
  return transform;
}
Square4 to_position_transform(Vec3 pos) {
  Square4 transform = eye<float, 4>();
  // fill position parameter
  for (size_t row = 0; row < 3; row++) {
    *(transform.begin() + 4 * row + 3) = *(pos.begin() + row);
  }
  return transform;
}
Square4 to_rotate_transform(Square3 rotate) {
  Square4 transform = eye<float, 4>();
  // fill ratate parameter
  for (size_t row = 0; row < 3; row++) {
    for (size_t col = 0; col < 3; col++) {
      *(transform.begin() + 4 * row + col) = *(rotate.begin() + 3 * row + col);
    }
  }
  return transform;
}