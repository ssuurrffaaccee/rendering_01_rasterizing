#pragma once
#include "clipping.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

class EnvironmentMappingReflectionShader : public Shader {
 public:
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
      const VertexStageData &v_2) override {
    Square4 &model_transform = *(Square4 *)get_global_data(0);
    std::vector<Plane> &clipping_planes =
        *(std::vector<Plane> *)get_global_data(1);
    auto world_v_0 = transform_to_world(model_transform, v_0);
    auto world_v_1 = transform_to_world(model_transform, v_1);
    auto world_v_2 = transform_to_world(model_transform, v_2);
    Square3 normal_matrix =
        transpose<float, 3>(inverse3(shrink(model_transform)));
    transform_norm_to_world_inplace(normal_matrix, world_v_0, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_1, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_2, 6);
    // std::cout<<"m"<<to_string(world_v_0)<<to_string(world_v_1)<<to_string(world_v_2)<<"\n";
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &projection_transform = *(Square4 *)get_global_data(3);
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
    // std::cout<<to_string(new_vertex)<<"\n";
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(4);
    Texture &environment_mapping = *(Texture *)get_global_data(5);
    // drop uv
    // float u = frag.second[4];
    // float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    auto reflecte_direction =
        reflect(camera_world_position - world_position, norm);
    Color color = environment_mapping.sample(
        reflecte_direction[0], reflecte_direction[1], reflecte_direction[2]);
    return std::make_tuple(frag.first, color, depth);
  }
};
REGISTER_SHADER(Shader, environment_mapping_reflection,
                EnvironmentMappingReflectionShader)

class EnvironmentMappingRefractionShader : public Shader {
 public:
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
      const VertexStageData &v_2) override {
    Square4 &model_transform = *(Square4 *)get_global_data(0);
    std::vector<Plane> &clipping_planes =
        *(std::vector<Plane> *)get_global_data(1);
    auto world_v_0 = transform_to_world(model_transform, v_0);
    auto world_v_1 = transform_to_world(model_transform, v_1);
    auto world_v_2 = transform_to_world(model_transform, v_2);
    Square3 normal_matrix =
        transpose<float, 3>(inverse3(shrink(model_transform)));
    transform_norm_to_world_inplace(normal_matrix, world_v_0, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_1, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_2, 6);
    // std::cout<<"m"<<to_string(world_v_0)<<to_string(world_v_1)<<to_string(world_v_2)<<"\n";
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &projection_transform = *(Square4 *)get_global_data(3);
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
    // std::cout<<to_string(new_vertex)<<"\n";
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(4);
    Texture &environment_mapping = *(Texture *)get_global_data(5);
    // drop uv
    // float u = frag.second[4];
    // float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    auto refraction_direction = refraction(
        normalize(camera_world_position - world_position), norm, 0.9f);//1.00 / 1.52 air to glass
    Color color = environment_mapping.sample(refraction_direction[0],
                                             refraction_direction[1],
                                             refraction_direction[2]);
    return std::make_tuple(frag.first, color, depth);
  }
};

REGISTER_SHADER(Shader, environment_mapping_refraction,
                EnvironmentMappingRefractionShader)