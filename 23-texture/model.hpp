#pragma once
#include <map>
#include <string>

#include "homo.hpp"
#include "mat.hpp"
#include "vec.hpp"
using TriangleAtrributes =
    std::tuple<std::vector<float>, std::vector<float>, std::vector<float>>;
template <typename T, size_t LEN>
void append(TriangleAtrributes& attrs, const Vec<T, LEN>& a,
            const Vec<T, LEN>& b, const Vec<T, LEN>& c) {
  auto& [a_attrs, b_attrs, c_attrs] = attrs;
  a_attrs.insert(a_attrs.end(), a.begin(), a.end());
  b_attrs.insert(b_attrs.end(), b.begin(), b.end());
  c_attrs.insert(c_attrs.end(), c.begin(), c.end());
}
struct Model {
  std::vector<Vec3> points_;
  std::vector<TriangleAtrributes> attributes_;
  std::vector<std::tuple<int, int, int>> triangles_;
};

struct Instance {
  std::string name_;
  Vec3 pos_;
  Vec3 scale_;
  Square3 rotate_;
  bool is_frame_;
  Square4 to_model_transform() const {
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
struct World {
  std::vector<Instance> instances_;
};

struct ModelPool {
  std::map<std::string, Model> models_;
};