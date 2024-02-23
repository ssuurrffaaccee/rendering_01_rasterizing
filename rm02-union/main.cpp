#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "sdf.hpp"

static const float MIN_DISTANCE{1e-5};

Color ray_marching(const Ray& ray, SignedDistanceField* sdf,
                   int max_marching_step) {
  float t = 0.0f;
  for (int i = 0; i < max_marching_step; i++) {
    Vec3 position = ray.at(t);
    float distance = sdf->distance(position);
    if (distance <= MIN_DISTANCE) {
      return abs(normalize(position));
      // return (normalize(position)+1.0f)*0.5f;
    }
    t = t + distance;
  }
  return Color{0.0f, 0.0f, 0.0f};
}

int main() {
  size_t height = 1000;
  size_t width = 1000;
  Image image{width, height};
  CameraConfig camera_config;
  Vec3 world_centor{0.0f, 0.0f, 0.0f};
  camera_config.origin_ = Vec3{0.0f, 0.0f, 3.0f};
  camera_config.forward_ = camera_config.origin_ - world_centor;
  camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
  camera_config.canvas_width_ = 1.0f;
  camera_config.canvas_height_ = 1.0f;
  camera_config.image_width_ = width;
  camera_config.image_height_ = height;
  camera_config.canvas_forward_distane_ = -1.0f;
  Camera camera;
  camera.init(camera_config);
  int max_marching_step = 5;
  auto sphere0 = make_sphere(Vec3{-0.5f, 0.0f, 0.0f}, 1.0f);
  auto sphere1 = make_sphere(Vec3{0.5f, 0.0f, 0.0f}, 1.0f);
  auto merged = make_union(sphere0, sphere1);
  auto ray_generator = camera.get_ray_generator();
  while (!ray_generator.empty()) {
    auto [pixel_pos, ray] = ray_generator.next();
    ray.direction_ = normalize(ray.direction_);
    Color color = ray_marching(ray, merged.get(), 10);
    image.set(pixel_pos[0], pixel_pos[1], color);
  }
  image.dump("xxxx");
  return 0;
}