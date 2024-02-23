#include <iostream>

#include "disk.hpp"
#include "image.hpp"
#include "ndc.hpp"
#include "shading.hpp"
MetaPoint<ColorProperty> naive_fragment_shader(
    const MetaPoint<ColorProperty> &meta_point, int height, int width) {
  Vec2 coord =
      Vec2{float(meta_point.pixel_pos[0]), float(meta_point.pixel_pos[1])} /
      Vec2{float(height), float(width)};
  coord = mod(coord * 8.0f, 1.0f);
  return MetaPoint<ColorProperty>{meta_point.pixel_pos,
                                  {Color{coord[0], coord[1], 0.0f}}};
}
int main() {
  try {
    size_t height = 1000;
    size_t width = 1000;
    Image image{width, height};
    Interval2D image_range = image.get_range();
    Shading shading{image_range};
    DiskGenerator disk{3};
    auto [points, triangles] = disk.get();
    for (auto &[a_idx, b_idx, c_idx] : triangles) {
      auto meta_points = shading.shading_triangle(
          MetaPoint<ColorProperty>{NDC_to_Image(points[a_idx], image_range),
                                   {Color{1.0f, 0.0f, 0.0f}}},
          MetaPoint<ColorProperty>{NDC_to_Image(points[b_idx], image_range),
                                   {Color{0.0f, 1.0f, 0.0f}}},
          MetaPoint<ColorProperty>{NDC_to_Image(points[c_idx], image_range),
                                   {Color{0.0f, 0.0f, 1.0f}}});
      for (auto &p : meta_points) {
        auto final_p = naive_fragment_shader(p, height, width);
        image.set(final_p.pixel_pos[0], final_p.pixel_pos[1],
                  final_p.property.color_);
      }
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}