#pragma once
#include <string>

#include "mat.hpp"
#include "meta_point.hpp"
#include "vec.hpp"
struct Model {
  std::vector<Vec3> points_;
  std::vector<Color> colors_;
  std::vector<std::tuple<int, int, int>> triangles_;
};
