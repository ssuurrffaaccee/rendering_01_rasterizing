#pragma once
#include "light.hpp"
#include "shader.hpp"
class BlinnPhongVertexShader : public VertexShader {
 public:
  BlinnPhongVertexShader() {}
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
    // intepolation world position
    new_vertex.insert(new_vertex.end(), homo_world_position.begin(),
                      homo_world_position.end() - 1);
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
class BlinnPhongFragmentShader : public FragmentShader {
 public:
  BlinnPhongFragmentShader() {}
  std::tuple<Vec<int, 2>, Color, float> process(const FragmentStageData &frag) {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(0);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(1);
    Color color = Color{frag.second[4], frag.second[5], frag.second[6]};
    Vec3 norm = Color{frag.second[7], frag.second[8], frag.second[9]};
    Vec3 world_position =
        Color{frag.second[10], frag.second[11], frag.second[12]};
    for (auto &light : lights) {
      final_color +=
          light->shading(world_position, norm, camera_world_position) * color;
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};