#pragma once
#include "camera.hpp"
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
Vec3 apply_transform(const Square4& transform, const Vec3& point) {
  auto homo_point = extend(point, 1.0f);
  auto transformed_homo_point = mvdot(transform, homo_point);
  transformed_homo_point[0] =
      transformed_homo_point[0] / transformed_homo_point[3];
  transformed_homo_point[1] =
      transformed_homo_point[1] / transformed_homo_point[3];
  return shrink(transformed_homo_point);
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