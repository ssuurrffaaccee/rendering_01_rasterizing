#pragma once
#include "clipping.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

class BlinnPhongShader : public Shader {
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
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
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
    // return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
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
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
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
    // std::cout<<THIS_LINE<<"\n";
    // std::cout<<"p"<<to_string(Vec3{frag.second[0],frag.second[1],frag.second[2]})<<"\n";
    for (auto &light : lights) {
      Color color =
          light->shading(world_position, norm, camera_world_position) *
          texture.sample(u, v);
      final_color += color;
      //* fabs(cos(length(5.0f*shrink(world_position))));
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, texture_blinn_phong, TextureBlinnPhongShader)

std::tuple<Vec3, Vec3, Vec3> compute_TBN(const VertexStageData &v_0,
                                         const VertexStageData &v_1,
                                         const VertexStageData &v_2,
                                         Vec3 norm) {
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
  return std::make_tuple(tangent, bitangent, norm);
  // return Square3{tangent[0], bitangent[0], norm[0],
  //                tangent[1], bitangent[1], norm[1],
  //                tangent[2], bitangent[2], norm[2]};
}
class NormTextureBlinnPhongShader : public Shader {
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
    Vec3 norm = Vec3{world_v_0[6], world_v_1[7], world_v_2[8]};

    auto [tangent, bitangent, nrom] =
        compute_TBN(world_v_0, world_v_1, world_v_2, norm);

    Vec3 T = normalize(mvdot(normal_matrix, tangent));   // tangent
    Vec3 B = normalize(mvdot(normal_matrix, bitangent)); // bitangent
    Vec3 N = normalize(mvdot(normal_matrix, norm));      // norm
    Square3 TBN;


    TBN[0] = T[0];
    TBN[3] = T[1];
    TBN[6] = T[2];
    TBN[1] = B[0];
    TBN[4] = B[1];
    TBN[7] = B[2];
    TBN[2] = N[0];
    TBN[5] = N[1];
    TBN[8] = N[2];
    Square3 *inverse_TBN = new Square3{};
    *inverse_TBN = transpose<float, 3>(TBN);
    set_global_data(8, (void *)inverse_TBN); // store inverse_TBN
    std::vector<std::shared_ptr<Light>> &world_lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    std::vector<std::shared_ptr<Light>> *tbn_lights =
        new std::vector<std::shared_ptr<Light>>{};
    for (auto &light : world_lights) {
      auto new_light = light->transform(*inverse_TBN);
      tbn_lights->push_back(new_light);
    }
    set_global_data(9, (void *)tbn_lights); // store TBN lights
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Vec3 *tbn_camera_world_position = new Vec3{};
    *tbn_camera_world_position = mvdot(*inverse_TBN, camera_world_position);
    set_global_data(
        10,
        (void *)tbn_camera_world_position); // store TBN camera world position
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
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
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
        u, v); // Vec3{frag.second[6], frag.second[7], frag.second[8]};
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
Vec2 parallax_mapping(Vec2 tex_coords, Vec3 view_dir,
                      Texture &displacement_texture) {
  float height_scale{0.035};
  float height = displacement_texture.sample(tex_coords[0], tex_coords[1])[0];
  float z = view_dir[2];
  Vec3 displacement = view_dir * height * height_scale;
  return tex_coords - Vec2{displacement[0] / z, displacement[1] / z};
}
class ParallaxMappingNormTextureBlinnPhongShader : public Shader {
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
    Vec3 norm = Vec3{world_v_0[6], world_v_1[7], world_v_2[8]};
    auto [tangent, bitangent, nrom] =
        compute_TBN(world_v_0, world_v_1, world_v_2, norm);
    Vec3 T = normalize(mvdot(normal_matrix, tangent));   // tangent
    Vec3 B = normalize(mvdot(normal_matrix, bitangent)); // bitangent
    Vec3 N = normalize(mvdot(normal_matrix, norm));      // norm
    Square3 TBN;
    TBN[0] = T[0];
    TBN[3] = T[1];
    TBN[6] = T[2];
    TBN[1] = B[0];
    TBN[4] = B[1];
    TBN[7] = B[2];
    TBN[2] = N[0];
    TBN[5] = N[1];
    TBN[8] = N[2];
    Square3 *inverse_TBN = new Square3{};
    *inverse_TBN = transpose<float, 3>(TBN);
    set_global_data(8, (void *)inverse_TBN); // store inverse_TBN
    std::vector<std::shared_ptr<Light>> &world_lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    std::vector<std::shared_ptr<Light>> *tbn_lights =
        new std::vector<std::shared_ptr<Light>>{};
    for (auto &light : world_lights) {
      auto new_light = light->transform(*inverse_TBN);
      tbn_lights->push_back(new_light);
    }
    set_global_data(9, (void *)tbn_lights); // store TBN lights
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Vec3 *tbn_camera_world_position = new Vec3{};
    *tbn_camera_world_position = mvdot(*inverse_TBN, camera_world_position);
    set_global_data(
        10,
        (void *)tbn_camera_world_position); // store TBN camera world position
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  ~ParallaxMappingNormTextureBlinnPhongShader() {
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
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &tbn_lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(9);
    Vec3 &tbn_camera_world_position = *(Vec3 *)get_global_data(10);
    Texture &texture = *(Texture *)get_global_data(6);
    Texture &norm_texture = *(Texture *)get_global_data(7);
    Texture &displace_texture = *(Texture *)get_global_data(11);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 tbn_world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    auto tex_coords = parallax_mapping(
        Vec2{u, v}, normalize(tbn_camera_world_position - tbn_world_position),
        displace_texture);
    u = tex_coords[0];
    v = tex_coords[1];
    if (u > 1.0 || v > 1.0 || u < 0.0 || v < 0.0) {
      u = frag.second[4];
      v = frag.second[5];
    }
    Vec3 tbn_norm = norm_texture.sample(
        u, v); // Vec3{frag.second[6], frag.second[7], frag.second[8]};
    tbn_norm = (tbn_norm - 0.5f) * 2.0f;
    auto texture_color = texture.sample(u, v);
    // std::cout<<"pos"<<to_string(tbn_world_position)<<"\n";
    // std::cout<<"norm"<<to_string(tbn_norm)<<"\n";
    // std::cout<<"came"<<to_string(tbn_camera_world_position)<<"\n";
    int count{0};
    for (auto &light : tbn_lights) {
      auto light_albedo = light->shading(tbn_world_position, tbn_norm,
                                         tbn_camera_world_position);
      //std::cout<<count<<" "<<to_string(light_albedo)<<"\n";
      count++;//why?????why?????
      final_color += light_albedo * texture_color;
      
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, parallax_mapping_norm_texture_blinn_phong,
                ParallaxMappingNormTextureBlinnPhongShader)
