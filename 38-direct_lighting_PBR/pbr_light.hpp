#pragma once
#include <cmath>
#include <memory>

#include "vec.hpp"
class PBRLight {
public:
  PBRLight() {}
  virtual ~PBRLight() {}
  virtual Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                        const Vec3 &camera_world_position, const Color &albedo,
                        float metallic, float roughness, float ao) = 0;
};

float distribution_GGX(const Vec3 &N, const Vec3 &H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = std::max(dot(N, H), 0.0f);
  float NdotH2 = NdotH * NdotH;

  float num = a2;
  float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
  denom = M_PI * denom * denom;

  return num / denom;
}

float geometry_schlick_GGX(float NdotV, float roughness) {
  float r = (roughness + 1.0f);
  float k = (r * r) / 8.0f;

  float num = NdotV;
  float denom = NdotV * (1.0f - k) + k;

  return num / denom;
}
float geometry_smith(const Vec3 &N, const Vec3 &V, const Vec3 &L,
                     float roughness) {
  float NdotV = std::max(dot(N, V), 0.0f);
  float NdotL = std::max(dot(N, L), 0.0f);
  float ggx2 = geometry_schlick_GGX(NdotV, roughness);
  float ggx1 = geometry_schlick_GGX(NdotL, roughness);

  return ggx1 * ggx2;
}
Vec3 fresnel_schlick(float cosTheta, const Vec3 &F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

class PBRPointLight : public PBRLight {
public:
  PBRPointLight() {}
  virtual ~PBRPointLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position, const Color &albedo,
                float metallic, float roughness, float ao) override {
    auto reverse_light_direction =
        normalize(position_ - fragment_world_position);
    auto reverse_view_direction =
        normalize(camera_world_position - fragment_world_position);
    Vec3 mid_dir = normalize(reverse_light_direction + reverse_view_direction);
    // calculate light radiance
    float distance = length(position_ - fragment_world_position);
    float attenuation = 1.0f / (distance * distance);
    Vec3 radiance = color_ * attenuation;
    //
    Vec3 F0 = Vec3{0.04f, 0.04f, 0.04f};
    F0 = mix(F0, albedo,metallic);
    // cook-torrance brdf
    float NDF = distribution_GGX(norm, mid_dir, roughness);
    float G = geometry_smith(norm, reverse_view_direction,
                             reverse_light_direction, roughness);
    Vec3 F = fresnel_schlick(
        std::max(dot(mid_dir, reverse_view_direction), 0.0f), F0);

    Vec3 kS = F;
    // std::cout<<to_string(kS)<<"\n";
    Vec3 kD = Vec3{1.0f, 1.0f, 1.0f} - kS;
    kD *= (1.0f - metallic);

    Vec3 nominator = NDF * G * F;
    float denominator = 4.0 *
                            std::max(dot(norm, reverse_view_direction), 0.0f) *
                            std::max(dot(norm, reverse_light_direction), 0.0f) +
                        0.001f;
    Vec3 specular = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = std::max(dot(norm, reverse_light_direction), 0.0f);
    // std::cout<<to_string(albedo)<<to_string(kD)<<to_string(specular)<<to_string(radiance)<<" "<<NdotL<<"\n";
    return (kD * albedo / M_PI + specular) * radiance * NdotL;
  }
  Color color_;
  Vec3 position_;
};
std::shared_ptr<PBRLight> make_pbr_point_light(const Vec3 &position,
                                               const Color &color) {
  PBRPointLight *point_light = new PBRPointLight{};
  point_light->color_ = color;
  point_light->position_ = position;
  return std::shared_ptr<PBRPointLight>((PBRPointLight *)point_light);
}
class PBRAmbientLight : public PBRLight {
public:
  PBRAmbientLight() {}
  virtual ~PBRAmbientLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position, const Color &albedo,
                float metallic, float roughness, float ao) override {
    return Vec3{0.03f, 0.03f, 0.03f} * albedo * ao;
  }
};
std::shared_ptr<PBRLight> make_pbr_ambient_light() {
  PBRAmbientLight *ambient_light = new PBRAmbientLight{};
  return std::shared_ptr<PBRLight>((PBRLight *)ambient_light);
}