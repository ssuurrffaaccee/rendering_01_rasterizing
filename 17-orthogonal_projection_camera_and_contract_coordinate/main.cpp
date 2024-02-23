#include <vector>

#include "base_indicator.hpp"
#include "cube.hpp"
#include "depth_image.hpp"
#include "render.hpp"
#include "sphere.hpp"
World build_world() {
  World world{};
  {
    SphereGenerator1 sphere_generator{3};
    Model sphere = sphere_generator.get();
    for (auto &p : sphere.points_) {
      sphere.colors_.push_back(abs(p));
    }

    world.models_["sphere"] = std::move(sphere);
    Model cube = get_cube();
    world.models_["cube"] = std::move(cube);
    world.models_["color_cube"] =
        get_single_color_cube(Color{1.0f, 1.0f, 1.0f});
    Model base_indicator = get_base_indicator();
    world.models_["base"] = std::move(base_indicator);
  }
  world.instances_.push_back(Instance{"base", Vec3{0.0f, 0.0f, 0.0f},
                                      Vec3{0.5f, 0.5f, 0.5f}, eye<float, 3>()});
  Square3 rotate{0.3333333,  -0.2440169, 0.9106836, 0.9106836, 0.3333333,
                 -0.2440169, -0.2440169, 0.9106836, 0.3333333};
  int seq01 = 0;
  for (float x = -0.5; x <= 0.5; x += 0.3) {
    for (float y = -0.5; y <= 0.5; y += 0.3) {
      seq01 = 1 - seq01;
      if (seq01 == 0) {
        world.instances_.push_back(Instance{"cube", Vec3{x, y, 0.0f},
                                            Vec3{0.25f, 0.25f, 0.25f},
                                            eye<float, 3>(), false});
        world.instances_.push_back(Instance{"color_cube", Vec3{x, y, 0.0f},
                                            Vec3{0.251f, 0.251f, 0.251f},
                                            eye<float, 3>(), true});

      } else {
        world.instances_.push_back(Instance{"color_cube", Vec3{x, y, 0.0f},
                                            Vec3{0.251f, 0.251f, 0.251f},
                                            eye<float, 3>(), true});
      }
    }
  }
  // world.instances_.push_back(Instance{"cube", Vec3{0.0f, 0.0f, 0.2f},
  //                                     Vec3{0.1f, 0.5f, 0.5f}, eye<float,
  //                                     3>()});

  return world;
}
int main() {
  try {
    Vec3 world_centor = Vec3{0.0f, 0.0f, 0.0f};
    size_t height = 1000;
    size_t width = 1000;
    OrthogonalCameraConfig camera_config;
    camera_config.position_ = Vec3{.7f, .7f, 1.0f};
    camera_config.forward_ = world_centor - camera_config.position_;
    camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
    camera_config.far_ = -0.5f;
    camera_config.near_ = 0.5f;
    camera_config.left_ = -0.5f;
    camera_config.right_ = 0.5f;
    camera_config.top_ = 0.5f;
    camera_config.down_ = -0.5f;
    OrthogonalCamera camera;
    camera.init(camera_config);
    DepthImage image{width, height};
    World world = build_world();
    for (auto &instance : world.instances_) {
      auto iter = world.models_.find(instance.name_);
      CHECK(iter != world.models_.end())
      render(iter->second,
             mmdot<float, 4>(camera.projection_transform_,
                             mmdot<float, 4>(camera.world_to_view_transform_,
                                             instance.to_model_transform())),
             image, instance.is_frame_);
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
}