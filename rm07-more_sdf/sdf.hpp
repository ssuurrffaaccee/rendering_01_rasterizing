#pragma once
#include "sptr.hpp"
#include "vec.hpp"
class SignedDistanceField {
 public:
  virtual ~SignedDistanceField() {}
  virtual float distance(const Vec3& position) = 0;
};
using SDF = SignedDistanceField;

class Union : public SDF {
 public:
  Union(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1)
      : sdf0_{sdf0}, sdf1_{sdf1} {}
  float distance(const Vec3& position) override {
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
  float distance(const Vec3& position) override {
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
  float distance(const Vec3& position) override {
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
  float distance(const Vec3& position) override {
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

class SphereSDF : public SDF {
 public:
  SphereSDF(const Vec3& origin, float radius)
      : origin_{origin}, radius_{radius} {}
  float distance(const Vec3& position) override {
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
class PlaneSDF : public SDF {
 public:
  PlaneSDF(const Vec3& sample_point, const Vec3& outside_direction)
      : sample_point_{sample_point}, outside_direction_{outside_direction} {
    outside_direction_ = normalize(outside_direction_);
  }
  float distance(const Vec3& position) override {
    return dot(position - sample_point_, outside_direction_);
  }

 private:
  Vec3 sample_point_;
  Vec3 outside_direction_;
};

SPtr<SignedDistanceField> make_plane_sdf(const Vec3& sample_point,
                                         const Vec3& outside_direction) {
  auto ptr = new PlaneSDF{sample_point, outside_direction};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class DisplaceSphereSDF : public SDF {
 public:
  DisplaceSphereSDF(const Vec3& origin, float radius)
      : origin_{origin}, radius_{radius} {}
  float distance(const Vec3& position) override {
    Vec3 offset = position - origin_;
    float displacement = sin(5.0f * offset[0]) * sin(5.0f * offset[1]) *
                         sin(5.0f * offset[2]) * 0.25f;
    return (length(position - origin_) - radius_) + displacement;
  }

 private:
  Vec3 origin_;
  float radius_;
};
SPtr<SignedDistanceField> make_displace_sphere_sdf(const Vec3& origin,
                                                   float radius) {
  auto ptr = new DisplaceSphereSDF{origin, radius};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

// float sdBox( vec3 p, vec3 b )
// {
//   vec3 q = abs(p) - b;
//   return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
// }

class BoxSDF : public SDF {
 public:
  BoxSDF(const Vec3& center, const Vec3& shape_size)
      : center_{center}, shape_size_{shape_size} {}
  float distance(const Vec3& position) override {
    Vec3 diff = abs(position - center_) - shape_size_;
    return length(max(diff, Vec3{0.0f, 0.0f, 0.0f})) +
           std::min(max(diff), 0.0f);
  }

 private:
  Vec3 center_;
  Vec3 shape_size_;
};

SPtr<SignedDistanceField> make_box_sdf(const Vec3& center, float length,
                                       float width, float height) {
  auto ptr = new BoxSDF{center, Vec3{length, height, width}};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class Round : public SDF {
 public:
  Round(const SPtr<SDF>& sdf, float r) : sdf_{sdf}, r_{r} {}
  float distance(const Vec3& position) override {
    return sdf_->distance(position) - r_;
  }

 private:
  SPtr<SDF> sdf_;
  float r_;
};

SPtr<SDF> make_round_sdf(const SPtr<SDF>& sdf0, float r) {
  auto ptr = new Round{sdf0, r};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class Onion : public SDF {
 public:
  Onion(const SPtr<SDF>& sdf, float r) : sdf_{sdf}, r_{r} {}
  float distance(const Vec3& position) override {
    return std::abs(sdf_->distance(position)) - r_;
  }

 private:
  SPtr<SDF> sdf_;
  float r_;
};

SPtr<SDF> make_onion_sdf(const SPtr<SDF>& sdf0, float r) {
  auto ptr = new Onion{sdf0, r};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

// float opSmoothUnion( float d1, float d2, float k )
// {
//     float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
//     return mix( d2, d1, h ) - k*h*(1.0-h);
// }

class SmoothUnion : public SDF {
 public:
  SmoothUnion(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1, float k)
      : sdf0_{sdf0}, sdf1_{sdf1}, k_{k} {}
  float distance(const Vec3& position) override {
    auto d0 = sdf0_->distance(position);
    auto d1 = sdf1_->distance(position);
    float h = clamp(0.5 + 0.5 * (d1 - d0) / k_, 0.0, 1.0);
    float res = mix(d1, d0, 1.0f - h) - k_ * h * (1.0 - h);
    return res;
  }

 private:
  SPtr<SDF> sdf0_;
  SPtr<SDF> sdf1_;
  float k_;
};

SPtr<SDF> make_smooth_union_sdf(const SPtr<SDF>& sdf0, const SPtr<SDF>& sdf1,
                                float k) {
  auto ptr = new SmoothUnion{sdf0, sdf1, k};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class InfiniteRepetition : public SDF {
 public:
  InfiniteRepetition(const SPtr<SDF>& sdf, const Vec3& repetition_size,
                     const Vec3& repetition_offset)
      : sdf_{sdf},
        repetition_size_{repetition_size},
        repetition_offset_{repetition_offset} {}
  float distance(const Vec3& position) override {
    auto p =
        (position - repetition_size_ * floor(position / repetition_size_)) +
        repetition_offset_;
    // std::cout<<to_string(p)<<"\n";
    return sdf_->distance(p);
  }

 private:
  SPtr<SDF> sdf_;
  Vec3 repetition_size_;
  Vec3 repetition_offset_;
};
SPtr<SDF> make_infinite_repetition_sdf(const SPtr<SDF>& sdf,
                                       const Vec3& repetition_size,
                                       const Vec3& repetition_offset) {
  auto ptr = new InfiniteRepetition{sdf, repetition_size, repetition_offset};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}

class InifiteOnionSphereSDF : public SDF {
 public:
  InifiteOnionSphereSDF(const Vec3& origin, float frequency)
      : origin_{origin}, frequency_{frequency} {}
  float distance(const Vec3& position) override {
    return sin(frequency_ * length(position - origin_));
  }

 private:
  Vec3 origin_;
  float frequency_;
};
SPtr<SignedDistanceField> make_infinite_onion_sphere_sdf(const Vec3& origin,
                                                         float frequency) {
  auto ptr = new InifiteOnionSphereSDF{origin, frequency};
  return SPtr<SignedDistanceField>{(SignedDistanceField*)ptr};
}
