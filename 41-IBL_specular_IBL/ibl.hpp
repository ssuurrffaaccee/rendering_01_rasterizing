#pragma once
#include <cmath>

#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

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
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
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
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
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
std::unique_ptr<ImageTexture> move_ssaa_depth_image_to_texture_image(
    SSAADepthImage *image) {
  auto texture = std::make_unique<ImageTexture>();
  texture->data_ = std::move(*image->data_);
  texture->width_ = image->SSAA_width_;
  texture->height_ = image->SSAA_height_;
  return texture;
}
std::shared_ptr<Texture> move_ssaa_depth_image_to_innner_texture_image(
    SSAADepthImage *image) {
  auto texture = std::make_shared<ImageTexture>();
  texture->data_ = std::move(*image->data_);
  texture->width_ = image->SSAA_width_;
  texture->height_ = image->SSAA_height_;
  return std::dynamic_pointer_cast<Texture>(texture);
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
  std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
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
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
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
    for (float phi = 0.0; phi < 2.0 * M_PI; phi += sample_delta)  // longitude
    {
      for (float theta = 0.0; theta < 0.5 * M_PI;
           theta += sample_delta)  // latitude
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

float radical_inverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10;  // / 0x100000000
}
Vec2 hammersley(uint i, uint N) {
  return Vec2{float(i) / float(N), radical_inverse_VdC(i)};
}
// float van_der_corpus(uint n, uint base)
// {
//     float invBase = 1.0f / float(base);
//     float denom   = 1.0f;
//     float result  = 0.0f;

//     for(uint i = 0u; i < 32u; ++i)
//     {
//         if(n > 0u)
//         {
//             denom   = std::fmod(float(n),2.0f);
//             result += denom * invBase;
//             invBase = invBase / 2.0f;
//             n       = uint(float(n) / 2.0f);
//         }
//     }

//     return result;
// }
// Vec2 HammersleyNoBitOps(uint i, uint N)
// {
//     return Vec2{float(i)/float(N), van_der_corpus(i, 2u)};
// }
Vec3 importance_sample_GGX(Vec2 Xi, Vec3 N, float roughness) {
  float a = roughness * roughness;

  float phi = 2.0 * M_PI * Xi[0];
  float cosTheta = sqrt((1.0 - Xi[1]) /
                        (1.0 + (a * a - 1.0) * Xi[1]));  // impotance sampling
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates
  Vec3 H;
  H[0] = cos(phi) * sinTheta;
  H[1] = sin(phi) * sinTheta;
  H[2] = cosTheta;

  // from tangent-space vector to world-space sample vector
  Vec3 up = abs(N[2]) < 0.999 ? Vec3{0.0f, 0.0f, 1.0f} : Vec3{1.0, 0.0, 0.0};
  Vec3 tangent = normalize(cross(up, N));
  Vec3 bitangent = cross(N, tangent);

  Vec3 sample_vec = tangent * H[0] + bitangent * H[1] + N * H[2];
  return normalize(sample_vec);
}
class IBLSpecularPrefilterMapCompution : public Shader {
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
  std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) override {
    float depth = frag.second[2];
    // float u = frag.second[4];
    // float v = frag.second[5];
    // Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Texture &cube_map = *(Texture *)get_global_data(4);
    float &roughness = *(float *)get_global_data(5);
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Vec3 out_direction = normalize(world_position);
    Vec3 R = out_direction;
    Vec3 V = R;  // view the ball in reverse out_direction

    const uint SAMPLE_COUNT = 1024u;
    float total_weight = 0.0;
    Vec3 prefiltered_color = gray(0.0);
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
      Vec2 Xi = hammersley(i, SAMPLE_COUNT);
      Vec3 H = importance_sample_GGX(Xi, out_direction, roughness);  // sampling
      Vec3 L =
          normalize(2.0 * dot(V, H) * H -
                    V);  // V reflected with norm H, L contribute to V's color
      // There are many simplification assumptions made in the code
      // implementation here. Just take a look at the implementation that is not
      // simplified in the following link
      // https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
      float NdotL = std::max(dot(out_direction, L), 0.0f);
      if (NdotL > 0.0) {
        prefiltered_color += cube_map.sample(L[0], L[1], L[2]) * NdotL;
        total_weight += NdotL;
      }
    }
    prefiltered_color =
        prefiltered_color /
        total_weight;  // Why not SAMPLE_COUNT.Because the inventor of the
                       // algorithm simplified and 'magic' it---try and write
    return std::make_tuple(frag.first, prefiltered_color, depth);
  }
};
REGISTER_SHADER(Shader, ibl_specular_prefilter_map_computation,
                IBLSpecularPrefilterMapCompution)

namespace ibl {
float geometry_schlick_GGX(float NdotV, float roughness) {
  // note that we use a different k for IBL
  float a = roughness;
  float k = (a * a) / 2.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float geometry_smith(const Vec3 &N, const Vec3 &V, const Vec3 &L,
                     float roughness) {
  float NdotV = std::max(dot(N, V), 0.0f);
  float NdotL = std::max(dot(N, L), 0.0f);
  float ggx2 = geometry_schlick_GGX(NdotV, roughness);
  float ggx1 = geometry_schlick_GGX(NdotL, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
Vec2 integrate_BRDF(float NdotV, float roughness) {
  if (NdotV == 0.0f) {
    NdotV = 0.00001f;
  }
  Vec3 V;
  V[0] = sqrt(1.0 - NdotV * NdotV);
  V[1] = 0.0;
  V[2] = NdotV;

  float A = 0.0;
  float B = 0.0;

  Vec3 N = Vec3{0.0, 0.0, 1.0};

  const uint SAMPLE_COUNT = 1024u;
  for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
    // generates a sample vector that's biased towards the
    // preferred alignment direction (importance sampling).
    Vec2 Xi = hammersley(i, SAMPLE_COUNT);
    Vec3 H = importance_sample_GGX(Xi, N, roughness);
    Vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = std::max(L[2], 0.0f);
    float NdotH = std::max(H[2], 0.0f);
    float VdotH = std::max(dot(V, H), 0.0f);

    if (NdotL > 0.0) {
      float G = geometry_smith(N, V, L, roughness);
      float G_Vis = (G * VdotH) / (NdotH * NdotV);
      float Fc = pow(1.0 - VdotH, 5.0);

      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  A /= float(SAMPLE_COUNT);
  B /= float(SAMPLE_COUNT);
  return Vec2{A, B};
}
}  // namespace ibl

class IBLSpecularBRDFCompution : public Shader {
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
    // std::cout<<to_string(frag.first)<<"\n";
    Vec2 uv = get_vec2_position(frag.second, 4);
    // std::cout<<to_string(uv)<<"\n";
    Vec2 result = ibl::integrate_BRDF(uv[0], uv[1]);
    // std::cout<<to_string(result)<<"\n";
    return std::make_tuple(frag.first, extend(result, 0.0), 0.0f);
  }
};
REGISTER_SHADER(Shader, ibl_specular_brdf_compution, IBLSpecularBRDFCompution)