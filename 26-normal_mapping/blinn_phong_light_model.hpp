#pragma once
#include "clipping.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

void transform_norm_to_world_inplace(const Square3 &normal_transform,
                                     VertexStageData &v, int start) {
  auto norm = Vec3{v[start], v[start + 1], v[start + 2]};
  auto world_norm = mvdot<float, 3>(normal_transform, norm);
  world_norm = normalize(world_norm);
  v[start] = world_norm[0];
  v[start + 1] = world_norm[1];
  v[start + 2] = world_norm[2];
}

class BlinnPhongShader : public Shader {
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
    transform_norm_to_world_inplace(normal_matrix, world_v_0, 7);
    transform_norm_to_world_inplace(normal_matrix, world_v_1, 7);
    transform_norm_to_world_inplace(normal_matrix, world_v_2, 7);
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
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
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
REGISTER_SHADER(Shader, blinn_phong, BlinnPhongShader)

class TextureBlinnPhongShader : public Shader {
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
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    for (auto &light : lights) {
      final_color +=
          light->shading(world_position, norm, camera_world_position) *
          texture.sample(u, v);
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, texture_blinn_phong, TextureBlinnPhongShader)

std::vector<Vec3> compute_TBN(const VertexStageData &v_0,
                              const VertexStageData &v_1,
                              const VertexStageData &v_2, Vec3 norm) {
  // Edge(3x2) = TB(3x2) * UV(2*2)
  // TB(3x2) = Edge(3x2) * inverse(UV(2*2));
  Vec3 pos_0 = get_vec3_position(v_0);
  Vec3 pos_1 = get_vec3_position(v_1);
  Vec3 pos_2 = get_vec3_position(v_2);
  Vec2 uv_0 = get_vec2_position(v_0, 4);
  Vec2 uv_1 = get_vec2_position(v_1, 4);
  Vec2 uv_2 = get_vec2_position(v_2, 4);
  Vec3 edge_1_0 = pos_0 - pos_1;
  Vec3 edge_1_2 = pos_2 - pos_1;
  Vec2 uv_1_0 = uv_0 - uv_1;
  Vec2 uv_1_2 = uv_2 - uv_1;
  Square2 UV{uv_1_0[0], uv_1_2[0], uv_1_0[1], uv_1_2[1]};
  auto inverse_UV = inverse2(UV);
  Vec2 invsere_uv_1_0 = Vec2{inverse_UV[0], inverse_UV[2]};
  Vec2 invsere_uv_1_2 = Vec2{inverse_UV[1], inverse_UV[3]};
  Vec3 tangent = edge_1_0 * invsere_uv_1_0[0] + edge_1_2 * invsere_uv_1_0[1];
  Vec3 bitangent = edge_1_0 * invsere_uv_1_2[0] + edge_1_2 * invsere_uv_1_2[1];
  tangent = normalize(tangent);
  bitangent = normalize(bitangent);
  std::vector<Vec3> res;
  res.push_back(tangent);
  res.push_back(bitangent);
  res.push_back(norm);
  return res;
}
class NormTextureBlinnPhongShader : public Shader {
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
    // Vec3 norm = Vec3{world_v_0[6], world_v_1[7], world_v_2[8]};
    Vec3 v0_norm = get_vec3_position(world_v_0, 6);
    Vec3 v1_norm = get_vec3_position(world_v_1, 6);
    Vec3 v2_norm = get_vec3_position(world_v_2, 6);
    Vec3 norm = v0_norm + v1_norm + v2_norm;
    std::vector<Vec3> TBN = compute_TBN(world_v_0, world_v_1, world_v_2, norm);
    Vec3 T = normalize(mvdot(normal_matrix, TBN[0]));  // tangent
    Vec3 B = normalize(mvdot(normal_matrix, TBN[1]));  // bitangent
    Vec3 N = normalize(mvdot(normal_matrix, TBN[2]));  // norm
    Square3 TBN_matrix =
        Square3{T[0], B[0], N[0], T[1], B[1], N[1], T[2], B[2], N[2]};
    Square3 *inverse_TBN = new Square3{};
    *inverse_TBN = transpose<float, 3>(TBN_matrix);
    set_global_data(8, (void *)inverse_TBN);  // store inverse_TBN
    std::vector<std::shared_ptr<Light>> &world_lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    std::vector<std::shared_ptr<Light>> *tbn_lights =
        new std::vector<std::shared_ptr<Light>>{};
    for (auto &light : world_lights) {
      auto new_light = light->transform(*inverse_TBN);
      tbn_lights->push_back(new_light);
    }
    set_global_data(9, (void *)tbn_lights);  // store TBN lights
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Vec3 *tbn_camera_world_position = new Vec3{};
    *tbn_camera_world_position = mvdot(*inverse_TBN, camera_world_position);
    set_global_data(
        10,
        (void *)tbn_camera_world_position);  // store TBN camera world position
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  ~NormTextureBlinnPhongShader() {
    Square3 *inverse_TBN = (Square3 *)get_global_data(8);
    delete inverse_TBN;
    std::vector<std::shared_ptr<Light>> *tangent_lights =
        (std::vector<std::shared_ptr<Light>> *)get_global_data(9);
    delete tangent_lights;
    Vec3 *tangent_camera_world_position = (Vec3 *)get_global_data(10);
    delete tangent_camera_world_position;
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
    Square3 &inverse_TBN = *(Square3 *)get_global_data(8);
    Vec3 world_position = Vec3{vertex[0], vertex[1], vertex[2]};
    Vec3 tbn_world_position =
        mvdot<float, 3>(inverse_TBN, Vec3{vertex[0], vertex[1], vertex[2]});
    // intepolation world position
    new_vertex.insert(new_vertex.end(), tbn_world_position.begin(),
                      tbn_world_position.end() - 1);
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
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &tbn_lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(9);
    Vec3 &tbn_camera_world_position = *(Vec3 *)get_global_data(10);
    Texture &texture = *(Texture *)get_global_data(6);
    Texture &norm_texture = *(Texture *)get_global_data(7);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 tbn_norm = norm_texture.sample(
        u, v);  // Vec3{frag.second[6], frag.second[7], frag.second[8]};
    tbn_norm = (tbn_norm - 0.5f) * 2.0f;
    Vec3 tbn_world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    for (auto &light : tbn_lights) {
      final_color += light->shading(tbn_world_position, tbn_norm,
                                    tbn_camera_world_position) *
                     texture.sample(u, v);
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, norm_texture_blinn_phong, NormTextureBlinnPhongShader)
