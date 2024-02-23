#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "sdf.hpp"

static const float MIN_DISTANCE{1e-5};

Color ray_marching(const Ray& ray, SDF* sdf, float marching_depth,
                   int max_marching_step) {
  float step_size = marching_depth / max_marching_step;
  Color final_color{0.0f, 0.0f, 0.0f};
  for (int i = 0; i < max_marching_step; i++) {
    float t = step_size * float(i);
    Vec3 position = ray.at(t);
    float density = sdf->get_density(position);
    final_color[0] += density*step_size;
  }
  return final_color;
}

int main() {
  size_t height = 1000;
  size_t width = 1000;
  Image image{width, height};
  CameraConfig camera_config;
  Vec3 world_centor{0.0f, 0.0f, 0.0f};
  camera_config.origin_ = Vec3{0.0f, 0.0f, 1.5f};
  camera_config.forward_ = camera_config.origin_ - world_centor;
  camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
  camera_config.canvas_width_ = 1.0f;
  camera_config.canvas_height_ = 1.0f;
  camera_config.image_width_ = width;
  camera_config.image_height_ = height;
  camera_config.canvas_forward_distane_ = -1.0f;
  Camera camera;
  camera.init(camera_config);
  int max_marching_step = 500;
  float marching_depth = 4.0f;
  auto sphere = make_sphere(Vec3{0.0f, 0.0f, 0.0f}, .5f);
  auto ray_generator = camera.get_ray_generator();
  while (!ray_generator.empty()) {
    auto [pixel_pos, ray] = ray_generator.next();
    ray.direction_ = normalize(ray.direction_);
    Color color =
        ray_marching(ray, sphere.get(), marching_depth, max_marching_step);
    image.set(pixel_pos[0], pixel_pos[1], color);
  }
  image.dump("xxxx");
  return 0;
}