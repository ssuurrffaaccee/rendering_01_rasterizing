#include <vector>

#include "depth_image.hpp"
#include "mat.hpp"
#include "meta_point.hpp"
#include "model.hpp"
#include "ndc.hpp"
#include "shading.hpp"
struct VertexData {
  Vec2 pos_;
  float depth_;
  Color color_;
};
// orthogonal projection
VertexData vertex_shader(const Vec3& point, const Color& color) {
  return VertexData{shrink(point), point[2], color};
}

void render(const Model& model, DepthImage& image, bool is_frame) {
  Shading shading{image.get_range()};
  auto image_range = image.get_range();
  std::vector<MetaPoint<ColorDepthProperty>> meta_points;
  meta_points.resize(model.points_.size());
  for (size_t i = 0; i < model.points_.size(); i++) {
    auto vertex_data = vertex_shader(model.points_[i], model.colors_[i]);
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
      image.set_with_depth_test(
          meta_points.pixel_pos[0], meta_points.pixel_pos[1],
          meta_points.property.color_, meta_points.property.depth_);
    }
  }
}

int main() {
  try {
    Model model;
    model.points_ = std::vector<Vec3>{
        {-0.25f, -0.25f, 0.0f}, {0.25f, -0.25f, 0.0f},  {0.25f, 0.25f, 0.0f},
        {-0.25f, 0.25f, 0.0f},  {-0.25f, -0.25f, 0.5f}, {0.25f, -0.25f, 0.5f},
        {0.25f, 0.25f, 0.5f},   {-0.25f, 0.25f, 0.5f}};
    model.colors_ = std::vector<Color>{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                       {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                                       {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    model.triangles_ = std::vector<std::tuple<int, int, int>>{
        {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
        {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
    Square3 rotate{0.3333333,  -0.2440169, 0.9106836, 0.9106836, 0.3333333,
                   -0.2440169, -0.2440169, 0.9106836, 0.3333333};
    for (auto& point : model.points_) {
      point = mvdot(rotate, (point - Vec3{0.0f, 0.0f, 0.25f})) +
              Vec3{0.0f, 0.0f, 0.25f};
    }
    DepthImage image{1000, 1000};
    render(model, image, false);
    model.colors_.clear();
    for (auto& _ : model.points_) {
      model.colors_.push_back(Color{1.0f, 1.0f, 1.0f});
    }
    render(model, image, true);
    image.dump("xxxx");
  } catch (MyExceptoin& e) {
    std::cout << e.what() << "\n";
  }
}