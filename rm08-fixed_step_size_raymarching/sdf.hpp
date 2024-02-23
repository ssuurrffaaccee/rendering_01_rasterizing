#pragma once
#include "sptr.hpp"
#include "vec.hpp"
class SignedDistanceField {
 public:
  virtual ~SignedDistanceField() {}
  virtual float distance(const Vec3& position) = 0;
  virtual float get_density(const Vec3& position) = 0;
};
using SDF = SignedDistanceField;
class Sphere : public SDF {
 public:
  Sphere(const Vec3& origin, float radius) : origin_{origin}, radius_{radius} {}
  float distance(const Vec3& position) override {
    return length(position - origin_) - radius_;
  }
  float get_density(const Vec3& position) override {
    auto dist = length(position - origin_);
    return dist > radius_ ? 0.0f : 2.0f*dist;
  }

 private:
  Vec3 origin_;
  float radius_;
};

SPtr<SignedDistanceField> make_sphere(const Vec3& origin, float radius) {
  auto ptr = new Sphere{origin, radius};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}