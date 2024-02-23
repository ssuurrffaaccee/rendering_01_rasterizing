#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

class ShadowMappingDepthMapShader : public Shader {
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
    //std::cout << "m" << to_string(world_v_0) << to_string(world_v_1)
    //          << to_string(world_v_2) << "\n";
    //return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &light_space_projection_transform = *(Square4 *)get_global_data(3);
    Vec4 homo_world_position{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_view = mvdot<float, 4>(view_transform, homo_world_position);
    //std::cout<<homo_world_position[0]<<" "<<homo_world_position[2]<<" "<<homo_world_position[3]<<"\n";
    auto p_in_projection =
        mvdot<float, 4>(light_space_projection_transform, p_in_view);
    new_vertex[0] = p_in_projection[0];
    new_vertex[1] = p_in_projection[1];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    //std::cout<<new_vertex[2]<<" "<<new_vertex[3]<<"\n";
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    return std::make_tuple(frag.first, Color{}, depth);
  }
};
REGISTER_SHADER(Shader, shadow_mapping_depth_map, ShadowMappingDepthMapShader)
// static SSAAOnlyDepthImage debug_shadow_map0{1000, 1000, 1};
// static SSAAOnlyDepthImage debug_shadow_map1{1000, 1000, 1};
bool calculate_shadow(const Square4 &view_and_projection_transform_with_direction_light,
                      const Vec3 &world_postiion,
                      const SSAAOnlyDepthImage &shadow_map) {
  auto p_in_projection = mvdot<float, 4>(view_and_projection_transform_with_direction_light,
                                         extend(world_postiion, 1.0f));
  auto fragment_depth = p_in_projection[2];
  auto width = shadow_map.SSAA_width_;
  auto height = shadow_map.SSAA_height_;
  auto image_coordnate =
      (Vec2{p_in_projection[0], p_in_projection[1]} + Vec2{1.0f, 1.0f}) * 0.5f;

  if (image_coordnate[0] < 0.0f || image_coordnate[0] >= 1.0f) {
    return false;
  }
  if (image_coordnate[1] < 0.0f || image_coordnate[1] >= 1.0f) {
    return false;
  }
  auto image_pos = Vec<int, 2>{int(width * image_coordnate[0]),
                               int(height * image_coordnate[1])};
  image_pos[1] = height - image_pos[1];  // flip y
  auto light_depth = shadow_map.get(image_pos[0], image_pos[1]);
  // debug_shadow_map0.set_with_depth_test(image_pos[0],image_pos[1],light_depth);
  // debug_shadow_map1.set_with_depth_test(image_pos[0],image_pos[1],fragment_depth);
  //std::cout<<light_depth<<" "<<fragment_depth<<"\n";
  return fragment_depth - light_depth < -0.009;
}
class ShadowMappingTextureBlinnPhongShader : public Shader {
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
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::shared_ptr<DirectionalLight> &directional_light =
        *(std::shared_ptr<DirectionalLight> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    Square4 &view_and_projection_transform_with_direction_light = *(Square4 *)get_global_data(7);
    SSAAOnlyDepthImage &shadow_map = *(SSAAOnlyDepthImage *)get_global_data(8);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    //std::cout<<to_string(world_position)<<"\n";
    bool shadowed = calculate_shadow(view_and_projection_transform_with_direction_light,
                                     world_position, shadow_map);
    // if(shadowed){
    //   final_color = Color{1.0f,0.0f,0.0f};
    // }else{
    //   final_color = Color{0.0f,1.0f,0.0f};
    // }
    // if(!shadowed){
    //   final_color += directional_light->shading(world_position, norm,
    //                                           camera_world_position) *
    //                texture.sample(u, v);
    // }
    final_color += directional_light->shading(world_position, norm,
                                              camera_world_position) *
                   texture.sample(u, v);
    final_color += Color{0.3,0.3,0.3} *
                   texture.sample(u, v);
    if(shadowed){
      final_color = final_color*0.5f;
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, shadow_mapping_texture_blinn_phong,
                ShadowMappingTextureBlinnPhongShader)