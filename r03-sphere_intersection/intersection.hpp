#pragma once
#include "ray.hpp"
struct IntersectionInfo {
  bool is_intersected_{false};
  float t_{0.0f};
  Color color_;
};
class Intersection {
 public:
  Intersection() {}
  virtual ~Intersection() {}
  virtual IntersectionInfo intersection(const Ray& ray) = 0;
};