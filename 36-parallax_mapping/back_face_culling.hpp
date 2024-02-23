#pragma once
#pragma once
#include "check.hpp"
#include "shader.hpp"
#include "vec.hpp"
//culling clockwise
static Vec3 camera_diretion = Vec3{0.0f, 0.0f, -1.0f};
bool is_back_face(const VertexStageData &data_a, const VertexStageData &data_b,
                  const VertexStageData &data_c) {
  Vec3 a = get_vec3_position(data_a);
  Vec3 b = get_vec3_position(data_b);
  Vec3 c = get_vec3_position(data_c);
  Vec3 ab = b - a;
  Vec3 bc = c - b;
  return dot(cross(ab, bc), camera_diretion) > 0.0f;
}