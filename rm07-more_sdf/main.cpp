#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "ray_tracing.hpp"
#include "sdf_wrapper.hpp"
void render(Camera& camera, Vec3 world_centor, size_t width, size_t height,
            SPtr<SDF>& sdf, const std::string& image_name) {
  Scene scene;
  scene.nodes_.push_back(make_sdf_wraper(sdf, Color{1.0f, 0.0f, 0.0f}, false));
  scene.lights_.push_back(make_directional_light(Vec3{-0.5f, -0.5f, 0.0f},
                                                 Color{1.0f, 1.0f, 1.0f}));
  scene.lights_.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
  // scene.lights_.push_back(make_point_light(
  //     world_centor + Vec3{0.0f, 2.0f, 0.0f}, Color{1.0f, 1.0f, 1.0f}));
  Range<float> range = Range<float>{1e-3, std::numeric_limits<float>::max()};
  int tracing_depth = 3;
  bool do_shadow = false;
  {
    Image image{width, height};
    auto ray_generator = camera.get_ray_generator();
    while (!ray_generator.empty()) {
      auto [pixel_pos, ray] = ray_generator.next();
      // ray.direction_ = ray.direction_*1e-2;
      auto color = ray_tracing(scene, ray, range, tracing_depth, do_shadow);
      image.set(pixel_pos[0], pixel_pos[1], color);
    }
    image.dump(image_name);
  }
}
int main() {
  Vec3 world_centor = Vec3{0.0f, 0.0f, -5.0f};
  size_t height = 1000;
  size_t width = 1000;
  CameraConfig camera_config;
  camera_config.origin_ = Vec3{3.0f, 3.0f, 3.0f};
  camera_config.forward_ = world_centor - camera_config.origin_;
  camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
  camera_config.canvas_width_ = 1.0f;
  camera_config.canvas_height_ = 1.0f;
  camera_config.image_width_ = width;
  camera_config.image_height_ = height;
  camera_config.canvas_forward_distane_ = 1.0f;
  Camera camera;
  camera.init(camera_config);
  {
    auto displace_sphere_sdf = make_displace_sphere_sdf(world_centor, 2.0f);
    render(camera, world_centor, width, height, displace_sphere_sdf, "xxx1");
  }
  {
    auto box_sdf = make_box_sdf(world_centor, 1.0f, 1.0f, 1.0f);
    render(camera, world_centor, width, height, box_sdf, "xxx2");
  }
  {
    auto box_sdf = make_box_sdf(world_centor, 1.0f, 1.0f, 1.0f);
    auto round_box_sdf = make_round_sdf(box_sdf, 0.2);
    render(camera, world_centor, width, height, round_box_sdf, "xxx3");
  }
  {
    auto sphere_sdf = make_sphere_sdf(world_centor, 2.0f);
    auto onion_sphere_sdf = make_onion_sdf(sphere_sdf, 0.2f);
    auto plane_sdf = make_plane_sdf(world_centor, Vec3{0.0, 1.0f, 0.0f});
    auto half_onion_sphere_sdf =
        make_intersection_sdf(onion_sphere_sdf, plane_sdf);
    render(camera, world_centor, width, height, half_onion_sphere_sdf, "xxx4");
  }
  {
    auto box_sdf = make_box_sdf(world_centor+Vec3{-1.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 1.0f);
    auto round_box_sdf = make_round_sdf(box_sdf, 0.2);
    auto sphere_sdf =
        make_sphere_sdf(world_centor + Vec3{1.4f, 0.0f, 0.0f}, 1.0f);
    auto smooth_union = make_smooth_union_sdf(round_box_sdf, sphere_sdf, 1.0f);
    render(camera, world_centor, width, height, smooth_union, "xxx5");
  }
  {
    auto sphere_sdf =
        make_sphere_sdf(Vec3{0.0f,0.0f,0.0f}, 0.25f);
    Vec3 repetition_size = Vec3{1.0f,1.0f,1.0f};
    Vec3 repetition_offset = repetition_size*(-0.5f);
    auto infinite_repetition =  make_infinite_repetition_sdf(sphere_sdf,repetition_size,repetition_offset);
    render(camera, world_centor, width, height, infinite_repetition, "xxx6");
  }
  {
    auto box_sdf = make_box_sdf(Vec3{0.0f,0.0f,0.0f}, 0.25f, 0.25f, 0.25f);
    auto round_box_sdf = make_round_sdf(box_sdf, 0.05);
    Vec3 repetition_size = Vec3{1.0f,1.0f,1.0f};
    Vec3 repetition_offset = repetition_size*(-0.5f);
    auto infinite_repetition =  make_infinite_repetition_sdf(round_box_sdf,repetition_size,repetition_offset);
    render(camera, world_centor, width, height, infinite_repetition, "xxx7");
  }
  {
    auto infinite_onion_sphere_sdf = make_infinite_onion_sphere_sdf(world_centor, 1.0f);
    auto box_sdf = make_box_sdf(world_centor, 1.0f, 1.0f, 0.0f);
    auto plane_sdf = make_plane_sdf(world_centor, Vec3{0.0, 1.0f, 0.0f});
    auto half_onion_sphere_sdf =
        make_intersection_sdf(infinite_onion_sphere_sdf, plane_sdf);
    render(camera, world_centor, width, height, half_onion_sphere_sdf, "xxx8");
  }
  return 0;
}