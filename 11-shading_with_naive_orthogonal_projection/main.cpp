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
void render(const Model& model, DepthImage& image) {
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
    std::vector<MetaPoint<ColorDepthProperty>> meta_points_after_fragment =
        shading.shading_triangle(meta_points[a_idx], meta_points[b_idx],
                                 meta_points[c_idx]);
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
    std::vector<Vec3> origin_triangle{
        {0.05f, 0.25f, 0.0f}, {0.05f, -0.25f, 0.0f}, {0.5f, -0.25f, 0.0f}};

    // Square3 yratate30{0.8660254, 0.0000000,  0.5000000, 0.0000000, 1.0000000,
    //                   0.0000000, -0.5000000, 0.0000000, 0.8660254};
    Square3 yratate30{0.8660254, 0.0000000, -0.5000000, 0.0000000, 1.0000000,
                      0.0000000, 0.5000000, 0.0000000,  0.8660254};
    for (auto& p : origin_triangle) {
      p = mvdot<float, 3>(yratate30, p);
    }
    Square3 zrotate120 = {-0.5000000, -0.8660254, 0.0000000,
                          0.8660254,  -0.5000000, 0.0000000,
                          0.0000000,  0.0000000,  1.0000000};

    Square3 zrotate240 = {-0.5000000, 0.8660254,  0.0000000,
                          -0.8660254, -0.5000000, 0.0000000,
                          0.0000000,  0.0000000,  1.0000000};

    Model model;
    model.points_.insert(model.points_.end(), origin_triangle.begin(),
                         origin_triangle.end());
    for (auto& p : origin_triangle) {
      model.points_.push_back(mvdot<float, 3>(zrotate120, p));
    }
    for (auto& p : origin_triangle) {
      model.points_.push_back(mvdot<float, 3>(zrotate240, p));
    }
    std::vector<Color> colors{
        {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    model.colors_.insert(model.colors_.end(), colors.begin(), colors.end());
    model.colors_.insert(model.colors_.end(), colors.begin(), colors.end());
    model.colors_.insert(model.colors_.end(), colors.begin(), colors.end());
    model.triangles_ =
        std::vector<std::tuple<int, int, int>>{{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
    DepthImage image{1000, 1000};
    render(model, image);
    image.dump("xxxx");
  } catch (MyExceptoin& e) {
    std::cout << e.what() << "\n";
  }
}