#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

class PointShadowMappingDepthMapShader : public Shader {
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
    // return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &light_space_projection_transform = *(Square4 *)get_global_data(3);
    Vec4 homo_world_position{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_view = mvdot<float, 4>(view_transform, homo_world_position);
    auto p_in_projection =
        mvdot<float, 4>(light_space_projection_transform, p_in_view);
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
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    // std::cout<<depth<<"\n";
    return std::make_tuple(frag.first, Color{}, depth);
  }
};
REGISTER_SHADER(Shader, point_shadow_mapping_depth_map,
                PointShadowMappingDepthMapShader)

struct CubeDepthCalculator {
  CubeDepthCalculator(std::vector<Square4> &view_and_projection_transforms)
      : view_and_projection_transforms_{
            std::move(view_and_projection_transforms)} {
    CHECK(view_and_projection_transforms_.size() == 6)
  }
  // face_dir:+x  left:-z right:+z  up:+y down:-y  right
  // face_dir:-x  left:+z right:-z  up:+y down:-y  left
  // face_dir:+z  left:+x right:-x  up:+y down:-y  front
  // face_dir:-z  left:-x right:+x  up:+y down:-y  back
  // face_dir:+y  left:+x right:-x  up:-z down:+z  up
  // face_dir:-y  left:+x right:-x  up:+z down:-z  down
  float cal_depth(float u, float v, float w, const Vec3 &world_position) {
    auto uvw = normalize(Vec3{u, v, w});
    u = uvw[0];
    v = uvw[1];
    w = uvw[2];
    int max_index = arg_abs_max(u, v, w);
    if (max_index == 0) {
      // CHECK(std::abs(u)==1.0f)
      if (u > 0.0f) {
        return cal_depth(0, u, v, w, world_position);
      }
      if (u <= 0.0f) {
        return cal_depth(1, u, v, w, world_position);
      }
    } else if (max_index == 1) {
      // y x z
      // CHECK(std::abs(v)==1.0f)
      if (v > 0.0f) {
        return cal_depth(2, u, v, w, world_position);
      }
      if (v <= -0.0f) {
        return cal_depth(3, u, v, w, world_position);
      }
    } else {
      // z x y
      // CHECK(std::abs(w)==1.0f)
      if (w > 0.0f) {
        return cal_depth(4, u, v, w, world_position);
      }
      if (w <= -0.0f) {
        return cal_depth(5, u, v, w, world_position);
      }
    }
    CHECK(false); // not reach
    return 0.0f;
  }
  float cal_depth(size_t index, float u, float v, float w,
                  const Vec3 &world_position) {
    auto p_in_projection = mvdot<float, 4>(
        view_and_projection_transforms_[index], extend(world_position, 1.0f));
    auto fragment_depth = p_in_projection[2];
    return fragment_depth;
  }
  std::vector<Square4> view_and_projection_transforms_;
};

bool calculate_shadow(CubeDepthCalculator &cube_depth_calculator,
                      const Vec3 &point_light_position,
                      const Vec3 &world_postiion,
                      OnlyDepthCubaImage &cube_shadow_map) {
  auto direction_on_point_light = world_postiion - point_light_position;
  auto fragment_depth = cube_depth_calculator.cal_depth(
      direction_on_point_light[0], direction_on_point_light[1],
      direction_on_point_light[2], world_postiion);
  auto light_depth = cube_shadow_map.get(direction_on_point_light[0],
                                         direction_on_point_light[1],
                                         direction_on_point_light[2]);
  // std::cout<<fragment_depth<<" "<<light_depth<<"\n";
  return fragment_depth - light_depth < -0.009;
}
class PointShadowMappingTextureBlinnPhongShader : public Shader {
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
    PointLight *point_light = (PointLight *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    CubeDepthCalculator &cube_depth_calculator =
        *(CubeDepthCalculator *)get_global_data(7);
    OnlyDepthCubaImage &cube_shadow_map =
        *(OnlyDepthCubaImage *)get_global_data(8);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    bool shadowed =
        calculate_shadow(cube_depth_calculator, point_light->position_,
                         world_position, cube_shadow_map);
    // if (shadowed) {
    //   final_color = Color{1.0f, 0.0f, 0.0f};
    // } else {
    //   final_color = Color{0.0f, 1.0f, 0.0f};
    // }
    final_color +=
        point_light->shading(world_position, norm, camera_world_position) *
        texture.sample(u, v);
    final_color += Color{0.3, 0.3, 0.3} * texture.sample(u, v);
    if (shadowed) {
      final_color = final_color * 0.5f;
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, point_shadow_mapping_texture_blinn_phong,
                PointShadowMappingTextureBlinnPhongShader)