#pragma once
#include "ray.hpp"
#include <limits>
struct IntersectionInfo {
  bool is_intersected_{false};
  float t_{std::numeric_limits<float>::max()};
  Color color_;
  Vec3 norm_;
};
class Intersection {
 public:
  Intersection() {}
  virtual ~Intersection() {}
  virtual IntersectionInfo intersection(const Ray& ray) = 0;
};