#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "ray_tracing.hpp"
#include "sdf_wrapper.hpp"
auto get_sdf0() {
  auto sphere0 = make_sphere_sdf(Vec3{-0.3f, 0.0f, -5.0f}, 1.0f);
  auto sphere1 = make_sphere_sdf(Vec3{0.3f, 0.0f, -5.0f}, 1.0f);
  auto sphere2 = make_sphere_sdf(Vec3{0.0f, 0.3f, -5.0f}, 1.0f);
  auto sphere3 = make_sphere_sdf(Vec3{0.0f, -0.3f, -5.0f}, 1.0f);
  auto merged0 = make_union_sdf(sphere0, sphere1);
  auto merged1 = make_union_sdf(sphere2, sphere3);
  auto inter = make_intersection_sdf(merged0, merged1);
  return inter;
}
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
  scene.nodes_.push_back(make_sdf_wraper(make_sphere_sdf(world_centor, 1.0f),
                                         Color{1.0f, 0.0f, 0.0f}, false));
  scene.nodes_.push_back(make_sdf_wraper(
      make_sphere_sdf(world_centor + Vec3{2.0f, 1.0f, 0.0f}, 1.0f),
      Color{0.0f, 1.0f, 0.0f}, true));
  scene.nodes_.push_back(make_sdf_wraper(
      make_sphere_sdf(world_centor + Vec3{-2.0f, 1.0f, 0.0f}, 1.0f),
      Color{0.0f, 0.0f, 1.0f}, false));
  float big_radius = 1000.0f;
  scene.nodes_.push_back(make_sdf_wraper(
      make_sphere_sdf(world_centor + Vec3{0.0f, -1.0f * big_radius, 0.0f},
                      big_radius),
      Color{0.0f, 0.6f, 1.0f}, false));

  // scene.nodes_.push_back(
  //     make_sphere(world_centor, 1.0f, Color{1.0f, 0.0f, 0.0f}, false));
  // scene.nodes_.push_back(make_sphere(world_centor + Vec3{2.0f, 1.0f, 0.0f},
  //                                    1.0f, Color{0.0f, 1.0f, 0.0f}, true));
  // scene.nodes_.push_back(make_sphere(world_centor + Vec3{-2.0f, 1.0f, 0.0f},
  //                                    1.0f, Color{0.0f, 0.0f, 1.0f}, false));
  // float big_radius = 1000.0f;
  // scene.nodes_.push_back(
  //     make_sphere(world_centor + Vec3{0.0f, -1.0f * big_radius, 0.0f},
  //                 big_radius, Color{0.0f, 0.6f, 1.0f}, false));

  scene.lights_.push_back(make_directional_light(Vec3{-0.5f, -0.5f, 0.0f},
                                                 Color{1.0f, 1.0f, 1.0f}));
  scene.lights_.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
  scene.lights_.push_back(make_point_light(
      world_centor + Vec3{0.0f, 2.0f, 0.0f}, Color{1.0f, 1.0f, 1.0f}));
  Range<float> range = Range<float>{1e-3, std::numeric_limits<float>::max()};
  int tracing_depth = 3;
  while (!ray_generator.empty()) {
    auto [pixel_pos, ray] = ray_generator.next();
    // ray.direction_ = ray.direction_*1e-2;
    auto color = ray_tracing(scene, ray, range, tracing_depth);
    image.set(pixel_pos[0], pixel_pos[1], color);
  }
  image.dump("xxxx");
  return 0;
}