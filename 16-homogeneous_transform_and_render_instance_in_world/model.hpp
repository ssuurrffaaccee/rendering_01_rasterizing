#pragma once
#include <map>
#include <string>

#include "mat.hpp"
#include "meta_point.hpp"
#include "vec.hpp"
struct Model {
  std::vector<Vec3> points_;
  std::vector<Color> colors_;
  std::vector<std::tuple<int, int, int>> triangles_;
};

struct Instance {
  std::string name_;
  Vec3 pos_;
  Vec3 scale_;
  Square3 rotate_;
  bool is_frame_;
  Square4 to_model_transform() {
    Square4 transform = eye<float, 4>();
    // fill ratate and scale parameter
    for (size_t row = 0; row < 3; row++) {
      for (size_t col = 0; col < 3; col++) {
        *(transform.begin() + 4 * row + col) =
            *(rotate_.begin() + 3 * row + col) * *(scale_.begin() + col);
      }
    }
    // fill position parameter
    for (size_t row = 0; row < 3; row++) {
      *(transform.begin() + 4 * row + 3) = *(pos_.begin() + row);
    }
    return transform;
  }
};

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
struct World {
  std::map<std::string, Model> models_;
  std::vector<Instance> instances_;
};