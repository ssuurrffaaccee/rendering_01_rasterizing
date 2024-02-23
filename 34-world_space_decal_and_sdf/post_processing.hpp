#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"
class InversionPostProcessing : public Shader {
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
    Color color = image.get(frag.first[0], frag.first[1]);
    final_color = Vec3{1.0f, 1.0f, 1.0f} - color;
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, inversion_post_processing, InversionPostProcessing)
class GrayscalePostProcessing : public Shader {
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
    Color color = image.get(frag.first[0], frag.first[1]);
    float average = 0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2];
    final_color = gray(average);
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, grayscale_post_processing, GrayscalePostProcessing)
class KernelPostProcessing : public Shader {
  static std::vector<int> offsets;

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
    std::vector<float> &weights = *(std::vector<float> *)get_global_data(1);
    for (int i = 0; i < 9; i++) {
      int offset_x = offsets[i];
      int offset_y = offsets[i + 1];
      auto x = clamp_int(frag.first[0] + offset_x, 0, image.SSAA_width_);
      auto y = clamp_int(frag.first[1] - offset_y, 0, image.SSAA_height_);
      final_color += image.get(x, y) * weights[i];
    }
    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
std::vector<int> KernelPostProcessing::offsets =
    std::vector<int>{-1, -1, /**/ 0, -1, /**/ 1, -1, /**/
                     -1, 0,  /**/ 0, 0,  /**/ 1, 0,  /**/
                     -1, 1,  /**/ 0, 1,  /**/ 1, 1,
                     /**/};
REGISTER_SHADER(Shader, kernel_post_processing, KernelPostProcessing)

static std::vector<float> sharpen_kernel{-1, -1, -1,                    /**/
                                         -1, 9,  -1,                    /**/
                                         -1, -1, -1};                   /**/
static std::vector<float> blur_kernel{1.0f / 16, 2.0f / 16, 1.0f / 16,  /**/
                                      2.0f / 16, 4.0f / 16, 2.0f / 16,  /**/
                                      1.0f / 16, 2.0f / 16, 1.0f / 16}; /**/
static std::vector<float> edge_detection_kernel{1, 1,  1,               /**/
                                                1, -8, 1,               /**/
                                                1, 1,  1};              /**/
