#include <vector>

#include "cube.hpp"
#include "depth_image.hpp"
#include "mat.hpp"
#include "meta_point.hpp"
#include "model.hpp"
#include "ndc.hpp"
#include "shading.hpp"
#include "sphere.hpp"
struct VertexData {
  Vec2 pos_;
  float depth_;
  Color color_;
};
// orthogonal projection
VertexData vertex_shader(const Vec3& point, const Color& color) {
  return VertexData{shrink(point), point[2], color};
}
Vec3 apply_transform(const Square4& transform, const Vec3& point) {
  auto homo_point = extend(point, 1.0f);
  return shrink(mvdot(transform, homo_point));
}
void render(const Model& model, const Square4& transform, DepthImage& image,
            bool is_frame) {
  Shading shading{image.get_range()};
  auto image_range = image.get_range();
  std::vector<MetaPoint<ColorDepthProperty>> meta_points;
  meta_points.resize(model.points_.size());
  for (size_t i = 0; i < model.points_.size(); i++) {
    auto vertex_data = vertex_shader(
        apply_transform(transform, model.points_[i]), model.colors_[i]);
    meta_points[i] = MetaPoint<ColorDepthProperty>{
        NDC_to_Image(vertex_data.pos_, image_range), vertex_data.depth_,
        vertex_data.color_};
  }
  for (auto [a_idx, b_idx, c_idx] : model.triangles_) {
    std::vector<MetaPoint<ColorDepthProperty>> meta_points_after_fragment;
    if (is_frame) {
      meta_points_after_fragment = shading.shading_triangle_frame(
          meta_points[a_idx], meta_points[b_idx], meta_points[c_idx]);
    } else {
      meta_points_after_fragment = shading.shading_triangle(
          meta_points[a_idx], meta_points[b_idx], meta_points[c_idx]);
    }
    for (auto& meta_points : meta_points_after_fragment) {
      // skip fragment shader
      CHECK(image_range.is_in(meta_points.pixel_pos[0],
                              meta_points.pixel_pos[1]));
      image.set_with_depth_test(
          meta_points.pixel_pos[0], meta_points.pixel_pos[1],
          meta_points.property.color_, meta_points.property.depth_);
    }
  }
}

World build_world() {
  World world{};
  {
    SphereGenerator1 sphere_generator{3};
    Model sphere = sphere_generator.get();
    for (auto& p : sphere.points_) {
      sphere.colors_.push_back(abs(p));
    }
    Model color_sphere = sphere;
    color_sphere.colors_.clear();
    for (auto& p : color_sphere.points_) {
      color_sphere.colors_.push_back(Color{1.0f, 1.0f, 1.0f});
    }

    world.models_["sphere"] = std::move(sphere);
    Model cube = get_cube();
    world.models_["cube"] = std::move(cube);
    world.models_["color_cube"] =
        get_single_color_cube(Color{1.0f, 1.0f, 1.0f});
    world.models_["color_sphere"] = color_sphere;
  }
  Square3 rotate{0.3333333,  -0.2440169, 0.9106836, 0.9106836, 0.3333333,
                 -0.2440169, -0.2440169, 0.9106836, 0.3333333};
  int seq = 0;
  for (float x = -0.5; x <= 0.5; x += 0.3) {
    for (float y = -0.5; y <= 0.5; y += 0.2) {
      int remainder = seq % 4;
      seq++;
      if (remainder == 0) {
        world.instances_.push_back(Instance{"cube", Vec3{x, y, 0.0f},
                                            Vec3{0.25f, 0.25f, 0.25f}, rotate,
                                            false});
        continue;
      }
      if (remainder == 1) {
        world.instances_.push_back(Instance{"color_cube", Vec3{x, y, 0.0f},
                                            Vec3{0.251f, 0.251f, 0.251f},
                                            rotate, true});
        continue;
      }
      if (remainder == 2) {
        world.instances_.push_back(Instance{"sphere", Vec3{x, y, 0.0f},
                                            Vec3{0.125f, 0.125f, 0.125f},
                                            rotate, false});
        continue;
      }
      if (remainder == 3) {
        world.instances_.push_back(Instance{"color_sphere", Vec3{x, y, 0.0f},
                                            Vec3{0.13f, 0.13f, 0.13f}, rotate,
                                            true});
        continue;
      }
    }
  }
  return world;
}
int main() {
  try {
    size_t height = 1000;
    size_t width = 1000;
    DepthImage image{width, height};
    World world = build_world();
    for (auto& instance : world.instances_) {
      auto iter = world.models_.find(instance.name_);
      CHECK(iter != world.models_.end())
      render(iter->second, instance.to_model_transform(), image,
             instance.is_frame_);
    }
    image.dump("xxxx");
  } catch (MyExceptoin& e) {
    std::cout << e.what() << "\n";
  }
}