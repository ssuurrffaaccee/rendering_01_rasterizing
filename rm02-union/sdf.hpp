#pragma once
#include "sptr.hpp"
#include "vec.hpp"
class SignedDistanceField {
 public:
  virtual ~SignedDistanceField() {}
  virtual float distance(const Vec3& position) = 0;
};
using SDF = SignedDistanceField;
class Sphere : public SDF {
 public:
  Sphere(const Vec3& origin, float radius) : origin_{origin}, radius_{radius} {}
  float distance(const Vec3& position) {
    return length(position - origin_) - radius_;
  }

 private:
  Vec3 origin_;
  float radius_;
};

SPtr<SignedDistanceField> make_sphere(const Vec3& origin, float radius) {
  auto ptr = new Sphere{origin, radius};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class Union : public SDF {
 public:
  Union(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1)
      : sdf0_{sdf0}, sdf1_{sdf1} {}
  float distance(const Vec3& position) {
    return std::min(sdf0_->distance(position), sdf1_->distance(position));
  }

 private:
  SPtr<SDF> sdf0_;
  SPtr<SDF> sdf1_;
};

SPtr<SDF> make_union(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1) {
  auto ptr = new Union{sdf0, sdf1};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}