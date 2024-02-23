#pragma once
#include "deferred_shading.hpp"
#include <random>
float lerp(float a, float b, float f) { return a + f * (b - a); }
std::vector<Vec3> generate_sample_kernel() {
  std::uniform_real_distribution<float> random_floats{0.0, 1.0};
  std::default_random_engine generator;
  std::vector<Vec3> ssao_kernel;
  for (unsigned int i = 0; i < 64; ++i) {
    // sample from [-1,1]x[-1,1]x[0,1]
    Vec3 sample{float(random_floats(generator)) * 2.0f - 1.0f,
                float(random_floats(generator)) * 2.0f - 1.0f,
                float(random_floats(generator))};
    sample = normalize(sample);
    sample *= float(random_floats(generator));
    float scale = float(i) / 64.0f;
    // 更多的注意放在靠近真正片段的遮蔽上
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale; // 将核心样本靠近原点分布
    ssao_kernel.push_back(sample);
  }
  return ssao_kernel;
}
// get random tangent in xy plane, norm is z; in tangent plane
std::shared_ptr<Texture> get_ssao_noise_texture() {
  std::uniform_real_distribution<float> random_floats{0.0, 1.0};
  std::default_random_engine generator;
  ScaleImageTexture *texture = new ScaleImageTexture{};
  texture->width_ = 4;
  texture->height_ = 4;
  texture->scale_ = 1;
  for (int i = 0; i < 16; i++) {
    Vec3 noise{float(random_floats(generator)) * 2.0f - 1.0f,
               float(random_floats(generator)) * 2.0f - 1.0f, 0.0f};
    texture->data_.push_back(noise);
  }
  return std::shared_ptr<Texture>{(Texture *)texture};
}
Vec<int, 2> from_projection_point_to_fragment_position(float x, float y,
                                                       int frame_width,
                                                       int frame_height) {
  Vec2 frame_size{float(frame_width) - 1, float(frame_height) - 1};
  Vec2 ndc_pos = clamp(Vec2{x, y}, -1.0f, 1.0f);
  // Vec2 ndc_pos = get_vec2_position(data);
  auto frame_pos = 0.5f * (ndc_pos + Vec2{1.0f, 1.0f}) * frame_size;
  frame_pos[1] = frame_size[1] - frame_pos[1]; // flip y
  Vec<int, 2> fragment_position;
  fragment_position[0] = int(frame_pos[0]);
  fragment_position[1] = int(frame_pos[1]);
  return fragment_position;
}
float smooth_step(float l, float r, float v) {
  float t = clamp((v - l) / (r - l), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}
float to_z_in_view(float depth, float camera_near, float camera_far) {
  return -((depth / (2.0f / (camera_near - camera_far))) +
           0.5f * (camera_near + camera_far));
}
class SSAOShader : public Shader {
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
    SSAAGBufferDepthImage &g_buffer_image =
        *(SSAAGBufferDepthImage *)get_global_data(0);
    Texture &noise_texture = *(Texture *)get_global_data(1);
    std::vector<Vec3> &samples = *(std::vector<Vec3> *)get_global_data(2);
    Square4 &view_transform = *(Square4 *)get_global_data(3);
    Square4 &projection_transform = *(Square4 *)get_global_data(4);
    float camera_near = *(float *)get_global_data(5);
    float camera_far = *(float *)get_global_data(6);
    auto g_buffer = g_buffer_image.get(frag.first[0], frag.first[1]);
    if (g_buffer.size() < 9) {
      return std::make_tuple(frag.first, gray(1.0f), 0.0f);
    }
    Vec3 norm = normalize(get_vec3_position(g_buffer, 3));
    Vec3 world_position = get_vec3_position(g_buffer, 6);
    Vec3 random_vec = noise_texture.sample((frag.first[0] % 4) / float(4),
                                           (frag.first[1] % 4) / float(4));
    Vec3 tangent = normalize(random_vec - norm * dot(random_vec, norm));
    Vec3 bitangent = cross(norm, tangent);
    Square3 inverse_TBN = Square3{tangent[0],   tangent[1],   tangent[2],
                                  bitangent[0], bitangent[1], bitangent[2],
                                  norm[0],      norm[1],      norm[2]};
    float occlusion{0.0f};
    for (int i = 0; i < kernel_size; ++i) {
      // get sample position
      Vec3 sample_pos =
          mvdot<float, 3>(inverse_TBN, samples[i]); // it's depth is kown.
      sample_pos = world_position + sample_pos * radius + norm * radius;
      Vec4 offset = extend(sample_pos, 1.0f);
      offset = mvdot<float, 4>(projection_transform,
                               mvdot<float, 4>(view_transform, offset));
      // float sample_depth = to_z_in_view(offset[2], -1.0f, -100.0f);
      float sample_depth = offset[3];
      offset = offset / offset[3];
      if (offset[0] >= 1.0f || offset[0] <= -1.0f || offset[1] >= 1.0f ||
          offset[1] <= -1.0f) {
        continue;
      }
      auto fragment_pos = from_projection_point_to_fragment_position(
          offset[0], offset[1], g_buffer_image.SSAA_width_,
          g_buffer_image.SSAA_height_);
      float visible_depth =
          g_buffer_image.get_depth(fragment_pos[0], fragment_pos[1]);
      visible_depth = to_z_in_view(visible_depth, camera_near, camera_far);
      // range check & accumulate
      float rangeCheck = smooth_step(
          0.0, 1.0,
          radius / abs(sample_depth -
                       visible_depth)); // sample_depth -
                                        // visible_depth is radius => 0
      occlusion += ((visible_depth <= sample_depth ? 1.0 : 0.0) * rangeCheck);
    }
    occlusion = 1.0 - (occlusion / kernel_size);
    return std::make_tuple(frag.first, gray(occlusion), 0.0f);
  }

private:
  // parameters (you'd probably want to use them as uniforms to more easily
  // tweak the effect)
  int kernel_size{64};
  float radius{0.5f};
  // float bias{0.025};
  float bias{0.025};

  // tile noise texture over screen based on screen dimensions divided by noise
  // size
  const Vec2 noise_scale = Vec2{800.0 / 4.0, 600.0 / 4.0};
};
SSAAGBufferDepthImage::GBuffer SSAOShader::default_g_buffer =
    std::vector<float>{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
REGISTER_SHADER(Shader, ssao, SSAOShader)

class SSAOBlur : public Shader {
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
    float result{0.0f};
    SSAADepthImage &ssao_image = *(SSAADepthImage *)get_global_data(0);
    for (int x = -2; x < 2; ++x) {
      for (int y = -2; y < 2; ++y) {
        auto x_pos = clamp_int(frag.first[0] + x, 0, ssao_image.SSAA_width_);
        auto y_pos = clamp_int(frag.first[1] + y, 0, ssao_image.SSAA_height_);
        result += ssao_image.get(x_pos, y_pos)[0];
      }
    }
    return std::make_tuple(frag.first, gray(result / (4.0 * 4.0)), 0.0f);
  }
};
REGISTER_SHADER(Shader, ssao_blur, SSAOBlur)