#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"
class BloomGenerateLightTextureBlinnPhongShader : public Shader {
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
    //  return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
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
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::shared_ptr<Light> &point_light =
        *(std::shared_ptr<Light> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    SSAADepthImage &bright_image = *(SSAADepthImage *)get_global_data(7);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Color color =
        point_light->shading(world_position, norm, camera_world_position) *
        texture.sample(u, v);
    final_color += color;
    float brightness = dot(final_color, Vec3{0.2126, 0.7152, 0.0722});
    if (brightness > 1.0f) {
      // warning: multi depth buffer
      bright_image.set_with_depth_test(frag.first[0], frag.first[1],
                                       final_color, depth);
    }else{
      bright_image.set_with_depth_test(frag.first[0], frag.first[1],
                                       Color{0.0f,0.0f,0.0f}, depth);//depth test
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, bloom_generate_light_texture_blinn_phong,
                BloomGenerateLightTextureBlinnPhongShader)
class BloomGenerateTextureBlinnPhongShader : public Shader {
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
    //  return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
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
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    SSAADepthImage &bright_image = *(SSAADepthImage *)get_global_data(7);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    for (auto &light : lights) {
      Color color =
          light->shading(world_position, norm, camera_world_position) *
          texture.sample(u, v);
      final_color += color;
    }
    float brightness = dot(final_color, Vec3{0.2126, 0.7152, 0.0722});
    if (brightness > 1.0f) {
      // warning: multi depth buffer
      bright_image.set_with_depth_test(frag.first[0], frag.first[1],
                                       final_color, depth);
    }else{
      bright_image.set_with_depth_test(frag.first[0], frag.first[1],
                                       Color{0.0f,0.0f,0.0f}, depth);//depth test
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, bloom_generate_texture_blinn_phong,
                BloomGenerateTextureBlinnPhongShader)
class BloomGuassBlur : public Shader {
  static std::array<float, 5> weights_;

 public:
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
      const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    SSAADepthImage &image = *(SSAADepthImage *)get_global_data(0);
    bool &is_horizontal = *(bool *)get_global_data(1);
    // auto color = image.get(frag.first[0], frag.first[1]) * weights_[0];
    final_color = image.get(frag.first[0], frag.first[1]) * weights_[0];
    // if (is_horizontal) {
    //   for (int i = 1; i < 5; ++i) {
    //     float horizontal_pixel_len = 1.0f/image.SSAA_width_;
    //     auto positive_x_offset =
    //         clamp_int(int(frag.first[0] + i*horizontal_pixel_len), 0, image.SSAA_width_);
    //     auto negative_x_offset =
    //         clamp_int(int(frag.first[0] - i*horizontal_pixel_len), 0, image.SSAA_width_);
    //     final_color +=
    //         image.get(positive_x_offset, frag.first[1]) * weights_[i];
    //     final_color +=
    //         image.get(negative_x_offset, frag.first[1]) * weights_[i];
    //   }
    // } else {
    //   for (int i = 1; i < 5; ++i) {
    //     float vertical_pixel_len = 1.0f/image.SSAA_height_;
    //     auto positive_y_offset =
    //         clamp_int(int(frag.first[1] + i*vertical_pixel_len), 0, image.SSAA_height_);
    //     auto negative_y_offset =
    //         clamp_int(int(frag.first[1] - i*vertical_pixel_len), 0, image.SSAA_height_);
    //     final_color +=
    //         image.get(frag.first[0], positive_y_offset) * weights_[i];
    //     final_color +=
    //         image.get(frag.first[0], negative_y_offset) * weights_[i];
    //   }
    // }
      if (is_horizontal) {
      for (int i = 1; i < 5; ++i) {
        auto positive_x_offset =
            clamp_int(frag.first[0] + i, 0, image.SSAA_width_);
        auto negative_x_offset =
            clamp_int(frag.first[0] - i, 0, image.SSAA_width_);
        final_color +=
            image.get(positive_x_offset, frag.first[1]) * weights_[i];
        final_color +=
            image.get(negative_x_offset, frag.first[1]) * weights_[i];
      }
    } else {
      for (int i = 1; i < 5; ++i) {
        auto positive_y_offset =
            clamp_int(frag.first[1] + i, 0, image.SSAA_height_);
        auto negative_y_offset =
            clamp_int(frag.first[1] - i, 0, image.SSAA_height_);
        final_color +=
            image.get(frag.first[0], positive_y_offset) * weights_[i];
        final_color +=
            image.get(frag.first[0], negative_y_offset) * weights_[i];
      }
    }
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
std::array<float, 5> BloomGuassBlur::weights_ =
    std::array<float, 5>{0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
REGISTER_SHADER(Shader, bloom_gauss_blur, BloomGuassBlur)

class BloomCombine : public Shader {
 public:
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
      const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    SSAADepthImage &color_image = *(SSAADepthImage *)get_global_data(0);
    SSAADepthImage &bright_image = *(SSAADepthImage *)get_global_data(1);
    float &exposure = *(float *)get_global_data(2);
    auto hdr_color = color_image.get(frag.first[0], frag.first[1]);
    auto bloom_color = bright_image.get(frag.first[0], frag.first[1]);
    final_color = hdr_color + bloom_color;
    // hdr
    final_color = Vec3{1.0f, 1.0f, 1.0f} - exp(-1.0f * final_color * exposure);
    const static float gamma = 2.2;
    const static float one_over_gamma = 1.0 / gamma;
    // gamma correct
    final_color =
        pow(final_color, Vec3{one_over_gamma, one_over_gamma, one_over_gamma});
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, bloom_combine, BloomCombine)