#pragma once
#include <cmath>
#include <memory>

#include "vec.hpp"
class Light {
public:
  virtual ~Light() {}
  virtual Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                        const Vec3 &camera_world_position) = 0;
  virtual std::shared_ptr<Light> transform(const Square3 transform) = 0;
};

float specular0(const Vec3 &reverse_view_direction,
                const Vec3 &reverser_light_direction, const Vec3 &norm,
                float shininess) {
  auto mid_dir = normalize(reverse_view_direction + reverser_light_direction);
  return std::pow(clamp(dot(norm, mid_dir), 0.0f, 1.0f), shininess);
}

Vec3 reflect(const Vec3 &reverse_view_direction, const Vec3 norm) {
  auto projection_on_norm = dot(reverse_view_direction, norm) * norm;
  return -1.0f * reverse_view_direction + 2.0f * projection_on_norm;
}

Vec3 refraction(const Vec3 &reverse_view_direction, const Vec3 norm,
                float ratio) {
  auto projection_on_norm = dot(reverse_view_direction, norm) * norm;
  auto perpendicular_to_norm =
      (projection_on_norm - reverse_view_direction) * ratio;
  auto perpendicular_to_norm_length = length(perpendicular_to_norm);
  return perpendicular_to_norm + sqrt(1.0f - perpendicular_to_norm_length *
                                                 perpendicular_to_norm_length) *
                                     (-1.0f * projection_on_norm);
}

float specular1(const Vec3 &reverse_view_direction,
                const Vec3 &reverser_light_direction, const Vec3 &norm,
                float shininess) {
  auto reflected_reverse_view_direction = reflect(reverse_view_direction, norm);
  return std::pow(
      clamp(dot(reverser_light_direction, reflected_reverse_view_direction),
            0.0f, 1.0f),
      shininess);
}

float diffuse(const Vec3 &reverser_light_direction, const Vec3 &norm) {
  // std::cout<<to_string(norm)<<to_string(reverser_light_direction)<<"\n";
  // std::cout<<dot(norm, reverser_light_direction)<<"\n";
  return clamp(dot(norm, reverser_light_direction), 0.0f, 1.0f);
}

class PointLight : public Light {
public:
  PointLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position) override {
    auto reverse_view_direction =
        normalize(camera_world_position - fragment_world_position);
    auto reverse_light_direction =
        normalize(position_ - fragment_world_position);
    float specular_value = specular0(reverse_light_direction,
                                     reverse_view_direction, norm, 100.0f);
    float diffuse_value = diffuse(reverse_light_direction, norm);
    // std::cout<<specular_value<<" "<<diffuse_value<<"\n";
    return (specular_value + diffuse_value) * color_;
  }
  std::shared_ptr<Light> transform(const Square3 transform) override {
    auto light = new PointLight{};
    light->position_ = mvdot<float, 3>(transform, position_);
    light->color_ = color_;
    return std::shared_ptr<Light>{(Light *)light};
  }
  Color color_;
  Vec3 position_;
};
std::shared_ptr<Light> make_point_light(const Vec3 &position,
                                        const Color &color) {
  PointLight *point_light = new PointLight{};
  point_light->color_ = color;
  point_light->position_ = position;
  return std::shared_ptr<Light>((Light *)point_light);
}
class AttenuationPointLight : public Light {
public:
  AttenuationPointLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position) override {
    auto reverse_view_direction =
        normalize(camera_world_position - fragment_world_position);
    auto reverse_light_direction =
        normalize(position_ - fragment_world_position);
    float specular_value = specular0(reverse_light_direction,
                                     reverse_view_direction, norm, 100.0f);
    float diffuse_value = diffuse(reverse_light_direction, norm);
    // std::cout<<specular_value<<" "<<diffuse_value<<"\n";
    float distance_to_light = length(fragment_world_position - position_);
    Color result_color = (specular_value + diffuse_value) * color_;
    float attenuation =
        1.0 / (constant_ + linear_ * distance_to_light +
               quadratic_ * (distance_to_light * distance_to_light));
    return result_color * attenuation;
  }
  std::shared_ptr<Light> transform(const Square3 transform) override {
    auto light = new PointLight{};
    light->position_ = mvdot<float, 3>(transform, position_);
    light->color_ = color_;
    return std::shared_ptr<Light>{(Light *)light};
  }
  Color color_;
  Vec3 position_;
  float constant_;
  float linear_;
  float quadratic_;
};
std::shared_ptr<Light> make_distance_point_light(const Vec3 &position,
                                                 const Color &color,
                                                 float constant, float linear,
                                                 float quadratic) {
  AttenuationPointLight *attenuation_point_light = new AttenuationPointLight{};
  attenuation_point_light->color_ = color;
  attenuation_point_light->position_ = position;
  attenuation_point_light->constant_ = constant;
  attenuation_point_light->linear_ = linear;
  attenuation_point_light->quadratic_ = quadratic;
  return std::shared_ptr<Light>((Light *)attenuation_point_light);
}
class DirectionalLight : public Light {
public:
  DirectionalLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position) override {
    auto reverse_view_direction =
        normalize(camera_world_position - fragment_world_position);
    auto reverse_light_direction = -1.0f * direction_;
    float specular_value = specular0(reverse_light_direction,
                                     reverse_view_direction, norm, 1000.0f);
    float diffuse_value = diffuse(reverse_light_direction, norm);
    return (specular_value + diffuse_value) * color_;
  }
  std::shared_ptr<Light> transform(const Square3 transform) override {
    auto light = new DirectionalLight{};
    light->direction_ = mvdot<float, 3>(transform, direction_);
    light->color_ = color_;
    return std::shared_ptr<Light>{(Light *)light};
  }
  Color color_;
  Vec3 direction_;
  Vec3 position_;
};
std::shared_ptr<Light> make_directional_light(const Vec3 &direction,
                                              const Color &color) {
  DirectionalLight *directional_light = new DirectionalLight{};
  directional_light->color_ = color;
  directional_light->direction_ = normalize(direction);
  directional_light->position_ = Vec3{0.0f, 0.0f, 0.0f};
  return std::shared_ptr<Light>((Light *)directional_light);
}
std::shared_ptr<DirectionalLight> make_directional_light(const Vec3 &direction,
                                                         const Color &color,
                                                         const Vec3 &position) {
  DirectionalLight *directional_light = new DirectionalLight{};
  directional_light->color_ = color;
  directional_light->direction_ = normalize(direction);
  directional_light->position_ = position;
  return std::shared_ptr<DirectionalLight>(directional_light);
}
class AmbientLight : public Light {
public:
  AmbientLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position) override {
    return color_;
  }
  std::shared_ptr<Light> transform(const Square3 transform) override {
    auto light = new AmbientLight{};
    light->color_ = color_;
    return std::shared_ptr<Light>{(Light *)light};
  }
  Color color_;
};
std::shared_ptr<Light> make_ambient_light(const Color &color) {
  AmbientLight *ambient_light = new AmbientLight{};
  ambient_light->color_ = color;
  return std::shared_ptr<Light>((Light *)ambient_light);
}