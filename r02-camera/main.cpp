#include <iostream>

#include "image.hpp"
#include "camera.hpp"

int main() {
  size_t height = 1000;
  size_t width = 1000;
  Image image{width, height};
  CameraConfig camera_config;
  camera_config.origin_ = Vec3{0.0f, 0.0f, 0.0f};
  camera_config.forward_ = Vec3{0.0f, 0.0f, 1.0f};
  camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
  camera_config.canvas_width_ = 1.0f;
  camera_config.canvas_height_ = 1.0f;
  camera_config.image_width_ = width;
  camera_config.image_height_ = height;
  camera_config.canvas_forward_distane_ = -1.0f;
  Camera camera;
  camera.init(camera_config);
  auto ray_generator = camera.get_ray_generator();
  while (!ray_generator.empty()) {
    auto [pixel_pos, ray] = ray_generator.next();
    Color color = normalize(ray.direction_);
    image.set(pixel_pos[0], pixel_pos[1], color);
  }
  image.dump("xxxx");
  return 0;
}