#pragma once
#include "sdf.hpp"
#include "vec.hpp"
static float normal_delta{1e-5};
Vec3 calculate_normal(const Vec3& position, SDF* sdf) {
  auto distance_x0 = sdf->distance(position - Vec3{normal_delta, 0.0f, 0.0f});
  auto distance_x1 = sdf->distance(position + Vec3{normal_delta, 0.0f, 0.0f});
  auto distance_y0 = sdf->distance(position - Vec3{0.0f, normal_delta, 0.0f});
  auto distance_y1 = sdf->distance(position + Vec3{0.0f, normal_delta, 0.0f});
  auto distance_z0 = sdf->distance(position - Vec3{0.0f, 0.0f, normal_delta});
  auto distance_z1 = sdf->distance(position + Vec3{0.0f, 0.0f, normal_delta});
  return normalize(Vec3{(distance_x1 - distance_x0) * 0.5f,
                        (distance_y1 - distance_y0) * 0.5f,
                        (distance_z1 - distance_z0) * 0.5f});
}