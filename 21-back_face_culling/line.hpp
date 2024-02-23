#pragma once
#include "vec.hpp"
// class Plane {
//  public:
//   Plane(const Vec3& sample_point, const Vec3& outside_direction)
//       : sample_point_{sample_point}, outside_direction_{outside_direction} {
//     outside_direction_ = normalize(outside_direction_);
//   }
//   bool is_inside(const Vec3& point) const { return distance(point) <= 0.0f; }
//   bool is_outside(const Vec3& point)const  { return !is_inside(point); }
//   float distance(const Vec3& point) const {
//     return dot(point - sample_point_, outside_direction_);
//   }
//   Vec3 sample_point_;
//   Vec3 outside_direction_;
// };

class Line {
 public:
  Line(const Vec2& sample_point, const Vec2& outside_direction)
      : sample_point_{sample_point}, outside_direction_{outside_direction} {
    outside_direction_ = normalize(outside_direction_);
  }
  bool is_inside(const Vec2& point) const { return distance(point) <= 0.0f; }
  bool is_outside(const Vec2& point) const  { return !is_inside(point); }
  float distance(const Vec2& point) const {
    return dot(point - sample_point_, outside_direction_);
  }
  Vec2 sample_point_;
  Vec2 outside_direction_;
};
