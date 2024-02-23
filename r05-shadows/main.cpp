#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "sphere.hpp"
// under scene.hpp
#include "light.hpp"

int main() {
  Vec3 world_centor = Vec3{0.0f, 0.0f, -5.0f};
  size_t height = 1000;
  size_t width = 1000;
  Image image{width, height};
  CameraConfig camera_config;
  camera_config.origin_ = Vec3{0.0f, 2.0f, 0.0f};
  camera_config.forward_ = world_centor - camera_config.origin_;
  camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
  camera_config.canvas_width_ = 1.0f;
  camera_config.canvas_height_ = 1.0f;
  camera_config.image_width_ = width;
  camera_config.image_height_ = height;
  camera_config.canvas_forward_distane_ = 1.0f;
  Camera camera;
  camera.init(camera_config);
  auto ray_generator = camera.get_ray_generator();
  Scene scene;
  scene.nodes_.push_back(
      make_sphere(world_centor, 1.0f, Color{1.0f, 0.0f, 0.0f}));
  scene.nodes_.push_back(make_sphere(world_centor + Vec3{2.0f, 1.0f, 0.0f},
                                     1.0f, Color{0.0f, 1.0f, 0.0f}));
  scene.nodes_.push_back(make_sphere(world_centor + Vec3{-2.0f, 1.0f, 0.0f},
                                     1.0f, Color{0.0f, 0.0f, 1.0f}));
  float big_radius = 1000.0f;
  scene.nodes_.push_back(
      make_sphere(world_centor + Vec3{0.0f, -1.0f * big_radius, 0.0f},
                  big_radius, Color{0.0f, 0.6f, 1.0f}));

  scene.lights_.push_back(make_directional_light(Vec3{-0.5f, -0.5f, 0.0f},
                                                 Color{1.0f, 1.0f, 1.0f}));
  scene.lights_.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
  scene.lights_.push_back(make_point_light(
      world_centor + Vec3{0.0f, 2.0f, 0.0f}, Color{1.0f, 1.0f, 1.0f}));
  Range<float> range = Range<float>{1e-17, std::numeric_limits<float>::max()};
  while (!ray_generator.empty()) {
    auto [pixel_pos, ray] = ray_generator.next();
    auto intersection_info = scene.intersection(ray, range);
    if (intersection_info.is_intersected_) {
      Color color = Color{0.0f, 0.0f, 0.0f};
      for (auto light : scene.lights_) {
        if (!light->is_shadow(intersection_info, ray, scene)) {
          color += light->shading(intersection_info, ray);
        }
      }
      image.set(pixel_pos[0], pixel_pos[1], color);
    } else {
      image.set(pixel_pos[0], pixel_pos[1], Color{0.1f, 0.1f, 0.1f});
    }
  }
  image.dump("xxxx");
  return 0;
}