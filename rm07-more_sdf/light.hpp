#pragma once
#include "intersection.hpp"
#include "range.hpp"
#include "ray.hpp"
#include "sptr.hpp"
#include "vec.hpp"
class Light {
public:
  virtual ~Light() {}
  virtual Color shading(const IntersectionInfo &intersetion_info,
                        const Ray &ray) = 0;
  virtual bool is_shadow(const IntersectionInfo &intersetion_info,
                         const Ray &ray, const Scene &scene) = 0;
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
  Color shading(const IntersectionInfo &intersetion_info,
                const Ray &ray) override {
    auto reverse_view_direction = -1.0f * ray.direction_;
    auto reverse_light_direction =
        normalize(position_ - ray.at(intersetion_info.t_));
    float specular_value =
        specular1(reverse_light_direction, reverse_view_direction,
                  intersetion_info.norm_, 1000.0f);
    float diffuse_value =
        diffuse(reverse_light_direction, intersetion_info.norm_);
    return (specular_value + diffuse_value) * intersetion_info.color_ * color_;
  }
  bool is_shadow(const IntersectionInfo &intersetion_info, const Ray &ray,
                 const Scene &scene) override {
    auto local_point = ray.at(intersetion_info.t_);
    auto detection_ray =
        Ray{ray.at(intersetion_info.t_ + 1e-17), position_ - local_point};
    auto detection_info =
        scene.intersection(ray, Range<float>{1e-17, effective_radius});
    return detection_info.is_intersected_;
  };
  float effective_radius{1.0f};
  Color color_;
  Vec3 position_;
};
SPtr<Light> make_point_light(const Vec3 &position, const Color &color) {
  PointLight *point_light = new PointLight{};
  point_light->color_ = color;
  point_light->position_ = position;
  return SPtr<Light>((Light *)point_light);
}
class DirectionalLight : public Light {
public:
  DirectionalLight() {}
  Color shading(const IntersectionInfo &intersetion_info,
                const Ray &ray) override {
    auto reverse_view_direction = -1.0f * ray.direction_;
    auto reverse_light_direction = -1.0f * direction_;
    float specular_value =
        specular0(reverse_light_direction, reverse_view_direction,
                  intersetion_info.norm_, 1000.0f);
    float diffuse_value =
        diffuse(reverse_light_direction, intersetion_info.norm_);
    return (specular_value + diffuse_value) * intersetion_info.color_ * color_;
  }
  bool is_shadow(const IntersectionInfo &intersetion_info, const Ray &ray,
                 const Scene &scene) override {
    auto detection_ray = Ray{ray.at(intersetion_info.t_), normalize(-1.0f*direction_)};
    auto detection_info = scene.intersection(detection_ray, Range<float>{1e-3f, std::numeric_limits<float>::max()});
    return detection_info.is_intersected_;
  };
  Color color_;
  Vec3 direction_;
};
SPtr<Light> make_directional_light(const Vec3 &direction, const Color &color) {
  DirectionalLight *directional_light = new DirectionalLight{};
  directional_light->color_ = color;
  directional_light->direction_ = normalize(direction);
  return SPtr<Light>((Light *)directional_light);
}
class AmbientLight : public Light {
public:
  AmbientLight() {}
  Color shading(const IntersectionInfo &intersetion_info,
                const Ray &ray) override {
    return color_;
  }
  bool is_shadow(const IntersectionInfo &intersetion_info, const Ray &ray,
                 const Scene &scene) override {
    return false;
  };
  Color color_;
};
SPtr<Light> make_ambient_light(const Color &color) {
  AmbientLight *ambient_light = new AmbientLight{};
  ambient_light->color_ = color;
  return SPtr<Light>((Light *)ambient_light);
}