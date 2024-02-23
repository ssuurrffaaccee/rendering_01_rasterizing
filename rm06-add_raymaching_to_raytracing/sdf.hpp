#pragma once
#include "sptr.hpp"
#include "vec.hpp"
class SignedDistanceField {
 public:
  virtual ~SignedDistanceField() {}
  virtual float distance(const Vec3& position) = 0;
};
using SDF = SignedDistanceField;
class SphereSDF : public SDF {
 public:
  SphereSDF(const Vec3& origin, float radius) : origin_{origin}, radius_{radius} {}
  float distance(const Vec3& position) {
    return length(position - origin_) - radius_;
  }

 private:
  Vec3 origin_;
  float radius_;
};

SPtr<SignedDistanceField> make_sphere_sdf(const Vec3& origin, float radius) {
  auto ptr = new SphereSDF{origin, radius};
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

SPtr<SDF> make_union_sdf(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1) {
  auto ptr = new Union{sdf0, sdf1};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class Intersetion : public SDF {
 public:
  Intersetion(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1)
      : sdf0_{sdf0}, sdf1_{sdf1} {}
  float distance(const Vec3& position) {
    return std::max(sdf0_->distance(position), sdf1_->distance(position));
  }

 private:
  SPtr<SDF> sdf0_;
  SPtr<SDF> sdf1_;
};

SPtr<SDF> make_intersection_sdf(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1) {
  auto ptr = new Intersetion{sdf0, sdf1};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}
class Inverse : public SDF {
 public:
  Inverse(const SPtr<SDF>& sdf) : sdf_{sdf} {}
  float distance(const Vec3& position) {
    return -1.0f * sdf_->distance(position);
  }

 private:
  SPtr<SDF> sdf_;
};

SPtr<SDF> make_inverse_sdf(const SPtr<SDF>& sdf0) {
  auto ptr = new Inverse{sdf0};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class Difference : public SDF {
 public:
  Difference(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1)
      : sdf0_{sdf0}, sdf1_{sdf1} {}
  float distance(const Vec3& position) {
    // inverse then intersection
    return std::max(sdf0_->distance(position),
                    -1.0f * sdf1_->distance(position));
  }

 private:
  SPtr<SDF> sdf0_;
  SPtr<SDF> sdf1_;
};

SPtr<SDF> make_difference_sdf(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1) {
  auto ptr = new Difference{sdf0, sdf1};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}
