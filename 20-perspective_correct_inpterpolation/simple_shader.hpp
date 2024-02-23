#pragma once
#include "check_texture.hpp"
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
    new_vertex[0] = p_in_projection[0];
    new_vertex[1] = p_in_projection[1];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    // perspective correct
    {
      float w = new_vertex[3];
      for (int i = 0; i < new_vertex.size(); i++) {
        new_vertex[i] /= w;
      }
      new_vertex[3] = 1.0f / w;
    }
    return new_vertex;
  }
};

class SimpleFragmentShader : public FragmentShader {
public:
  SimpleFragmentShader() {}
  std::tuple<Vec<int, 2>, Color, float> process(const FragmentStageData &frag) override {
    CheckerTexture &check_texture = *(CheckerTexture *)get_global_data(0);
    float depth = frag.second[2];
    float u = frag.second[4];
    float v = frag.second[5];
    Color color = check_texture.sample(u, v);
    return std::make_tuple(frag.first, color, depth);
  }
};