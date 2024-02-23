#pragma once
#include "range.hpp"
#include "scene.hpp"
#include "sphere.hpp"
// under scene.hpp
#include "light.hpp"
Color ray_tracing(const Scene &scene, const Ray &ray, const Range<float> &range,
                  size_t tracing_depth) {
  Ray cur_ray = ray;
  Color color = Color{0.0f, 0.0f, 0.0f};
  IntersectionInfo intersection_info;
  for (int bounce = 0; bounce < tracing_depth; bounce++) {
    intersection_info = scene.intersection(cur_ray, range);
    if (!intersection_info.is_intersected_) {
      return color;
    }
    for (auto light : scene.lights_) {
      if (!light->is_shadow(intersection_info, cur_ray, scene)) {
        auto c = light->shading(intersection_info, cur_ray);
        color += c;        
      }
    }
    if (!intersection_info.is_reflective_) {
      return color;
    }
    auto next_ray_direction =
        reflect(-1.0f * cur_ray.direction_, intersection_info.norm_);
    cur_ray = Ray{cur_ray.at(intersection_info.t_), next_ray_direction};
  }
  return Color{0.1f, 0.1f, 0.1f};
}