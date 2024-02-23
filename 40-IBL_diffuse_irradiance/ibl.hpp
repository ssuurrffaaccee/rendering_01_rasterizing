#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <cmath>

// std::atan2 : [  , ]->[-pi  ,  pi]
// std::asin  : [-1,1]->[-pi/2,pi/2]
// const Vec2 inv_atan = Vec2{0.1591, 0.3183};
// const Vec2 inv_atan = Vec2{1.0f / (2.0f * M_PI), 1.0f / (2.0f * M_PI)};
// Vec2 sample_spherical_map(const Vec3 &v) {
//   Vec2 uv = Vec2{std::atan2(v[2], v[0]), std::asin(v[1])};
//   uv *= inv_atan;
//   uv += 1.0f;
//   uv *= 0.5f;
//   return uv;
// }
// Vec2 sample_spherical_map(const Vec3 &v) {
//   float x = std::atan2(v[2], v[0]);
//   float y = std::asin(v[1]);
//   x = ((x / M_PI) + 1.0f) * 0.5;
//   y = ((y / (2.0f * M_PI)) + 1.0f) * 0.5;
//   return Vec2{x, y};
// }
Vec2 sample_spherical_map(const Vec3 &v) {
  float x = std::atan2(v[2], v[0]);
  float y = std::asin(v[1]);
  x = ((x / M_PI) + 1.0f) * 0.5;
  y = (y * 2.0f / M_PI + 1.0f) * 0.5;
  return Vec2{x, y};
}
class IBLEquidistantCylindricalToCubeMapping : public Shader {
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
    // float u = frag.second[4];
    // float v = frag.second[5];
    // Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Texture &equidistant_cylindrical_texture = *(Texture *)get_global_data(4);
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Vec2 uv = sample_spherical_map(normalize(world_position));
    auto color = equidistant_cylindrical_texture.sample(uv[0], uv[1]);
    return std::make_tuple(frag.first, color, depth);
  }
};
REGISTER_SHADER(Shader, ibl_equidistant_cylindrical_to_cube_mapping,
                IBLEquidistantCylindricalToCubeMapping)
std::unique_ptr<ImageTexture>
move_ssaa_depth_image_to_texture_image(SSAADepthImage *image) {
  auto texture = std::make_unique<ImageTexture>();
  texture->data_ = std::move(*image->data_);
  texture->width_ = image->SSAA_width_;
  texture->height_ = image->SSAA_height_;
  return texture;
}
std::shared_ptr<Texture> six_image_to_cub_mapping_texture(
    std::vector<std::unique_ptr<SSAADepthImage>> &images) {
  std::vector<std::unique_ptr<ImageTexture>> texture_inners;
  for (auto &image : images) {
    texture_inners.push_back(
        move_ssaa_depth_image_to_texture_image(image.get()));
  }
  auto cube_texture_concrete = std::make_shared<CubeMapTexture>(texture_inners);
  auto cube_texture = std::dynamic_pointer_cast<Texture>(cube_texture_concrete);
  return cube_texture;
}

class IBLIrradianceConvolutionPrecompution : public Shader {
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
    // std::cout<<to_string(frag.first)<<"\n";
    float depth = frag.second[2];
    // float u = frag.second[4];
    // float v = frag.second[5];
    // Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Texture &cube_map = *(Texture *)get_global_data(4);
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Vec3 out_direction = normalize(world_position);

    Vec3 irradiance = gray(0.0f);
    // tangent space calculation from origin point
    Vec3 up = Vec3{0.0, 1.0, 0.0};
    Vec3 right = normalize(cross(up, out_direction));
    up = normalize(cross(out_direction, right));
    float sample_delta{0.025};
    float nr_samples{0.0};
    // 将每一个位置(phi,theta)看做有一个理想点光源,光源强度为从贴图中采样的颜色值.
    for (float phi = 0.0; phi < 2.0 * M_PI; phi += sample_delta) // longitude
    {
      for (float theta = 0.0; theta < 0.5 * M_PI;
           theta += sample_delta) // latitude
      {
        // spherical to cartesian (in tangent space)
        Vec3 tangent_sample =
            Vec3{sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)};
        // tangent space to world
        Vec3 sample_vec = tangent_sample[0] * right + tangent_sample[1] * up +
                          tangent_sample[2] * out_direction;

        irradiance +=
            cube_map.sample(sample_vec[0], sample_vec[1], sample_vec[2]) *
            cos(theta) * sin(theta);
        nr_samples++;
      }
    }
    irradiance = M_PI * irradiance * (1.0 / float(nr_samples));
    return std::make_tuple(frag.first, irradiance, depth);
  }
};
REGISTER_SHADER(Shader, ibl_irradiance_convolution_precompution,
                IBLIrradianceConvolutionPrecompution)