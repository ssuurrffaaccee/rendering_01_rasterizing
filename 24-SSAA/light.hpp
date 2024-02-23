#pragma once
#include "vec.hpp"
#include <cmath>
#include <memory>
class Light {
public:
  virtual ~Light() {}
  virtual Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                        const Vec3 &camera_world_position) = 0;
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
    float specular_value = specular1(reverse_light_direction,
                                     reverse_view_direction, norm, 1000.0f);
    float diffuse_value = diffuse(reverse_light_direction, norm);
    return (specular_value + diffuse_value) * color_;
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
  Color color_;
  Vec3 direction_;
};
std::shared_ptr<Light> make_directional_light(const Vec3 &direction,
                                              const Color &color) {
  DirectionalLight *directional_light = new DirectionalLight{};
  directional_light->color_ = color;
  directional_light->direction_ = normalize(direction);
  return std::shared_ptr<Light>((Light *)directional_light);
}
class AmbientLight : public Light {
public:
  AmbientLight() {}
  Color shading(const Vec3 &fragment_world_position, const Vec3 &norm,
                const Vec3 &camera_world_position) override {
    return color_;
  }
  Color color_;
};
std::shared_ptr<Light> make_ambient_light(const Color &color) {
  AmbientLight *ambient_light = new AmbientLight{};
  ambient_light->color_ = color;
  return std::shared_ptr<Light>((Light *)ambient_light);
}