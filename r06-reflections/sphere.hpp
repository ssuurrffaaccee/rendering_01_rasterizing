#pragma once
#include "intersection.hpp"
#include "ray.hpp"
#include "sptr.hpp"
#include "vec.hpp"
class Sphere : public Intersection {
public:
  Sphere(const Vec3 &centor, const float radius, const Color &color,
         const bool is_reflective)
      : centor_{centor}, radius_{radius}, color_{color},
        is_reflective_{is_reflective} {}
  Sphere() {}
  IntersectionInfo intersection(const Ray &ray,
                                const Range<float> &range) const override {
    IntersectionInfo intersect_info;
    intersect_info.color_ = color_;
    intersect_info.is_reflective_ = is_reflective_;
    Vec3 ray_dir = ray.direction_;
    float a = dot(ray_dir, ray_dir);
    Vec3 centor_to_origin = ray.origin_ - centor_;
    float b = 2.0f * dot(centor_to_origin, ray_dir);
    float c = dot(centor_to_origin, centor_to_origin) - radius_ * radius_;
    float d = b * b - 4 * a * c;
    if (d < EPS) {
      intersect_info.is_intersected_ = false;
      return intersect_info;
    }
    float t0 = 0.5 * (1 / a) * (-1.0f * b + sqrt(d));
    float t1 = 0.5 * (1 / a) * (-1.0f * b - sqrt(d));
    if (t0 < EPS && t1 < EPS) {
      intersect_info.is_intersected_ = false;
      return intersect_info;
    }
    if (t0 - t1 > EPS) {
      std::swap(t0, t1);
    }
    bool is_in = range.is_in(t0);
    if (is_in) {
      intersect_info.t_ = t0;
      intersect_info.norm_ = norm(ray.at(intersect_info.t_));
      intersect_info.is_intersected_ = is_in;
      return intersect_info;
    }
    is_in = range.is_in(t1);
    if (is_in) {
      intersect_info.t_ = t1;
      intersect_info.norm_ = norm(ray.at(intersect_info.t_));
      intersect_info.is_intersected_ = is_in;
      return intersect_info;
    }
    intersect_info.is_intersected_ = false;
    return intersect_info;
  }
  Vec3 norm(const Vec3 &point) const { return normalize(point - centor_); }
  Vec3 centor_{0.0f, 0.0f, 0.0f};
  float radius_{1.0f};
  Color color_;
  bool is_reflective_;
};

SPtr<Intersection> make_sphere(const Vec3 &centor, float radius,
                               const Color &color, bool is_reflective) {
  Sphere *sphere = new Sphere{centor, radius, color, is_reflective};
  return std::shared_ptr<Intersection>((Intersection *)sphere);
}