#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "pbr_light.hpp"
#include "shader.hpp"
#include "texture.hpp"
class PBRDirectLightDeferredGeometryPassShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
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
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    SSAAGBufferDepthImage &g_buffer_image =
        *(SSAAGBufferDepthImage *)get_global_data(4);
    Vec3 &albedo = *(Vec3 *)get_global_data(5);
    float &metallic = *(float *)get_global_data(6);
    float &roughness = *(float *)get_global_data(7);
    float &ao = *(float *)get_global_data(8);
    // float u = frag.second[4];
    // float v = frag.second[5];
    SSAAGBufferDepthImage::GBuffer g_buffer;
    g_buffer.resize(3 + 3 + 3 + 3);
    g_buffer[0] = albedo[0];
    g_buffer[1] = albedo[1];
    g_buffer[2] = albedo[2];
    // norm
    g_buffer[3] = frag.second[6];
    g_buffer[4] = frag.second[7];
    g_buffer[5] = frag.second[8];
    // position
    g_buffer[6] = frag.second[9];
    g_buffer[7] = frag.second[10];
    g_buffer[8] = frag.second[11];
    // metallic roughness ao
    g_buffer[9] = metallic;
    g_buffer[10] = roughness;
    g_buffer[11] = ao;
    g_buffer_image.set_with_depth_test(frag.first[0], frag.first[1], g_buffer,
                                       depth);
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, pbr_directed_light_deferred_geometry,
                PBRDirectLightDeferredGeometryPassShader)

class PBRDeferredLightPassShader : public Shader {
  static SSAAGBufferDepthImage::GBuffer default_g_buffer;

public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<PBRLight>> &lights =
        *(std::vector<std::shared_ptr<PBRLight>> *)get_global_data(0);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(1);
    SSAAGBufferDepthImage &g_buffer_image =
        *(SSAAGBufferDepthImage *)get_global_data(2);
    auto g_buffer = g_buffer_image.get(frag.first[0], frag.first[1]);
    if (g_buffer.size() < 12) {
      return std::make_tuple(frag.first, final_color, 0.0f);
    }
    Vec3 albedo = get_vec3_position(g_buffer, 0);
    Vec3 norm = get_vec3_position(g_buffer, 3);
    norm = normalize(norm);
    Vec3 world_position = get_vec3_position(g_buffer, 6);
    Vec3 pbr_parameter = get_vec3_position(g_buffer, 9);
    float metallic = pbr_parameter[0];
    float roughness = pbr_parameter[1];
    float ao = pbr_parameter[2];
    for (auto &light : lights) {
      auto light_shading =
          light->shading(world_position, norm, camera_world_position, albedo,
                         metallic, roughness, ao);
      // std::cout<<to_string(light_shading)<<"\n";
      final_color += light_shading;
    }
    // gamma correction
    final_color = final_color / (final_color + gray(1.0f));
    final_color = pow(final_color, gray(1.0f / 2.2f));
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
SSAAGBufferDepthImage::GBuffer PBRDeferredLightPassShader::default_g_buffer =
    std::vector<float>{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
REGISTER_SHADER(Shader, pbr_deferred_light, PBRDeferredLightPassShader)