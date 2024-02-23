#pragma once
#include "intersection.hpp"
#include "normal.hpp"
#include "ray.hpp"
#include "sdf.hpp"
#include "sptr.hpp"
#include "vec.hpp"
static const float MIN_DISTANCE{1e-5};
// static const float MIN_DELTA_DISTANCE{1e-3};
static const int MAX_MARCHING_STEP{8};
std::pair<bool, float> ray_marching(const Ray &ray, SignedDistanceField *sdf,
                                    int max_marching_step) {
  float t = 0.0f;
  float distance = std::numeric_limits<float>::max();
  for (int i = 0; i < max_marching_step; i++) {
    Vec3 position = ray.at(t);
    auto new_distance = sdf->distance(position);
    // if( distance*distance - new_distance*distance < MIN_DELTA_DISTANCE){
    //   return std::pair<bool, float>{true, t};
    // }
    distance = new_distance;
    if (distance <= MIN_DISTANCE) {
      return std::pair<bool, float>{true, t};
    }
    t = t + distance;
  }
  return std::pair<bool, float>{false, 0.0f};
}
class SDFWrapper : public Intersection {
public:
  SDFWrapper(const SPtr<SDF> sdf, const Color &color, const bool is_reflective)
      : sdf_{sdf}, color_{color}, is_reflective_{is_reflective} {}
  IntersectionInfo intersection(const Ray &ray,
                                const Range<float> &range) const override {
    IntersectionInfo intersect_info;
    intersect_info.color_ = color_;
    intersect_info.is_reflective_ = is_reflective_;
    auto [is_intersected, t] = ray_marching(ray, sdf_.get(), MAX_MARCHING_STEP);
    is_intersected = is_intersected && range.is_in(t);
    intersect_info.is_intersected_ = is_intersected;
    if (is_intersected) {
      intersect_info.t_ = t;
      auto pos = ray.at(t);
      intersect_info.norm_ = calculate_normal(pos, sdf_.get());
    }
    return intersect_info;
  }

private:
  SPtr<SDF> sdf_;
  Color color_;
  bool is_reflective_;
};

SPtr<Intersection> make_sdf_wraper(const SPtr<SDF> sdf, const Color &color,
                                   bool is_reflective) {
  auto *ptr = new SDFWrapper{sdf, color, is_reflective};
  return std::shared_ptr<Intersection>((Intersection *)ptr);
}

