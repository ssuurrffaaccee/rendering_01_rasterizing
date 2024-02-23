#pragma once
#include "shader.hpp"

class SimpleVertexShader : public VertexShader {
 public:
  SimpleVertexShader() {}
  VertexStageData process(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(0);
    Square4 &projection_transform = *(Square4 *)get_global_data(1);
    Vec4 homo_world_position{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_view = mvdot<float, 4>(view_transform, homo_world_position);
    auto p_in_projection = mvdot<float, 4>(projection_transform, p_in_view);
    new_vertex[0] = p_in_projection[0] / p_in_projection[3];
    new_vertex[1] = p_in_projection[1] / p_in_projection[3];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    return new_vertex;
  }
};

class SimpleFragmentShader : public FragmentShader {
 public:
  SimpleFragmentShader() {}
  std::tuple<Vec<int, 2>, Color, float> process(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color color = Color{frag.second[4], frag.second[5], frag.second[6]};
    return std::make_tuple(frag.first, color, depth);
  }
};