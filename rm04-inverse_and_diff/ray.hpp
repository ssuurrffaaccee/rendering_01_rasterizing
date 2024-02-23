#pragma once
#include "vec.hpp"

class Ray {
public:
  explicit Ray(const Vec3 &origin, const Vec3 &direction)
      : origin_{origin}, direction_{direction} {}
  Vec3 at(float t) const  { return origin_ + t * direction_; }
  Vec3 origin_;
  Vec3 direction_;
};