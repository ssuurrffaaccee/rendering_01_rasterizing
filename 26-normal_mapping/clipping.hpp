#pragma once
#include "check.hpp"
#include "plane.hpp"
#include "shader.hpp"
#include "vec.hpp"
#include <iostream>

std::vector<VertexStageData> clipping(const VertexStageData &a,
                                      const VertexStageData &b,
                                      const VertexStageData &c,
                                      const Plane &plane) {
  std::vector<VertexStageData> generated_triangles;
  auto a_position = get_vec3_position(a);
  auto b_position = get_vec3_position(b);
  auto c_position = get_vec3_position(c);
  bool is_a_inside = plane.is_inside(a_position);
  bool is_b_inside = plane.is_inside(b_position);
  bool is_c_inside = plane.is_inside(c_position);

  if (is_a_inside && is_b_inside && is_c_inside) {
    generated_triangles.push_back(a);
    generated_triangles.push_back(b);
    generated_triangles.push_back(c);
    return generated_triangles;
  }
  if (!is_a_inside && !is_b_inside && !is_c_inside) {
    return generated_triangles;
  }
  float a_distance = std::abs(plane.distance(a_position));
  float b_distance = std::abs(plane.distance(b_position));
  float c_distance = std::abs(plane.distance(c_position));
  if (is_a_inside && !is_b_inside && !is_c_inside) {
    float mix_ab = a_distance / (a_distance + b_distance);
    float mix_ac = a_distance / (a_distance + c_distance);
    auto new_b = ::mix(a, b, (1.0f - mix_ab));
    auto new_c = ::mix(a, c, (1.0f - mix_ac));
    generated_triangles.push_back(a);
    generated_triangles.push_back(new_b);
    generated_triangles.push_back(new_c);
    return generated_triangles;
  }
  if (!is_a_inside && is_b_inside && !is_c_inside) {
    float mix_ba = b_distance / (b_distance + a_distance);
    float mix_bc = b_distance / (b_distance + c_distance);
    auto new_a = ::mix(b, a, (1.0f - mix_ba));
    auto new_c = ::mix(b, c, (1.0f - mix_bc));
    generated_triangles.push_back(new_a);
    generated_triangles.push_back(b);
    generated_triangles.push_back(new_c);
    return generated_triangles;
  }
  if (!is_a_inside && !is_b_inside && is_c_inside) {
    float mix_ca = c_distance / (c_distance + a_distance);
    float mix_cb = c_distance / (c_distance + b_distance);
    auto new_a = ::mix(c, a, (1.0f - mix_ca));
    auto new_b = ::mix(c, b, (1.0f - mix_cb));
    generated_triangles.push_back(new_a);
    generated_triangles.push_back(new_b);
    generated_triangles.push_back(c);
    return generated_triangles;
  }
  if (!is_a_inside && is_b_inside && is_c_inside) {
    float mix_ba = b_distance / (a_distance + b_distance);
    float mix_ca = c_distance / (a_distance + c_distance);
    auto new_ba = ::mix(b, a, (1.0f - mix_ba));
    auto new_ca = ::mix(c, a, (1.0f - mix_ca));
    generated_triangles.push_back(b);
    generated_triangles.push_back(c);
    generated_triangles.push_back(new_ca);
    generated_triangles.push_back(new_ca);
    generated_triangles.push_back(new_ba);
    generated_triangles.push_back(b);
    return generated_triangles;
  }
  if (is_a_inside && !is_b_inside && is_c_inside) {
    float mix_ab = a_distance / (b_distance + a_distance);
    float mix_cb = c_distance / (b_distance + c_distance);
    auto new_ab = ::mix(a, b, (1.0f - mix_ab));
    auto new_cb = ::mix(c, b, (1.0f - mix_cb));
    generated_triangles.push_back(c);
    generated_triangles.push_back(a);
    generated_triangles.push_back(new_ab);
    generated_triangles.push_back(new_ab);
    generated_triangles.push_back(new_cb);
    generated_triangles.push_back(c);
    return generated_triangles;
  }
  if (is_a_inside && is_b_inside && !is_c_inside) {
    float mix_ac = a_distance / (c_distance + a_distance);
    float mix_bc = b_distance / (c_distance + b_distance);
    auto new_ac = ::mix(a, c, (1.0f - mix_ac));
    auto new_bc = ::mix(b, c, (1.0f - mix_bc));
    generated_triangles.push_back(a);
    generated_triangles.push_back(b);
    generated_triangles.push_back(new_bc);
    generated_triangles.push_back(new_bc);
    generated_triangles.push_back(new_ac);
    generated_triangles.push_back(a);
    return generated_triangles;
  }
  CHECK(false)
  return generated_triangles;
}
std::vector<VertexStageData> clipping(
    const VertexStageData &a, const VertexStageData &b,
    const VertexStageData &c, const std::vector<Plane> &clipping_planes) {
  std::vector<VertexStageData> old_generated_triangles{a, b, c};
  std::vector<VertexStageData> new_generated_triangles;
  for (auto plane : clipping_planes) {
    for (int i = 2; i < old_generated_triangles.size(); i += 3) {
      auto temp_triangles = clipping(old_generated_triangles[i - 2],
                                     old_generated_triangles[i - 1],
                                     old_generated_triangles[i], plane);
      new_generated_triangles.insert(new_generated_triangles.end(),
                                     temp_triangles.begin(),
                                     temp_triangles.end());
    }
    std::swap(old_generated_triangles, new_generated_triangles);
    new_generated_triangles.clear();
  }
  return old_generated_triangles;
}

VertexStageData transform_to_world(const Square4 &world_transform,
                                   const VertexStageData &v) {
  auto homo_point = get_vec4_position(v);
  auto world_homo_point = mvdot<float, 4>(world_transform, homo_point);
  auto new_v = v;
  new_v[0] = world_homo_point[0];
  new_v[1] = world_homo_point[1];
  new_v[2] = world_homo_point[2];
  new_v[3] = world_homo_point[3];
  return new_v;
}

// class ClippingTriangleGeometryShader : public GeometryShader {
//  public:
//   ClippingTriangleGeometryShader() {}
//   std::vector<VertexStageData> process(const VertexStageData &v_0,
//                                        const VertexStageData &v_1,
//                                        const VertexStageData &v_2) override {
//     Square4 &model_transform = *(Square4 *)get_global_data(0);
//     std::vector<Plane> &clipping_planes =
//         *(std::vector<Plane> *)get_global_data(1);
//     auto world_v_0 = transform_to_world(model_transform, v_0);
//     auto world_v_1 = transform_to_world(model_transform, v_1);
//     auto world_v_2 = transform_to_world(model_transform, v_2);
//     // std::cout<<"m"<<to_string(world_v_0)<<to_string(world_v_1)<<to_string(world_v_2)<<"\n";
//     return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
//   }
// };

// REGISTER_SHADER(geometry, GeometryShader, clipping_triangle,
//                 ClippingTriangleGeometryShader)
