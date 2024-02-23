#include <iostream>

#include "image.hpp"
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
    Shading shading{image.get_range()};
    // triangle 0
    MetaPoint<ColorProperty> a{{10, 10}, {0.0f, 0.0f, 0.0f}};
    MetaPoint<ColorProperty> b{{999, 10}, {0.0f, 1.0f, 0.0f}};
    MetaPoint<ColorProperty> c{{10, 999}, {0.0f, 0.0f, 1.0f}};
    MetaPoint<ColorProperty> d{{999, 999}, {1.0f, 0.0f, 0.0f}};
    {
      auto meta_points = shading.shading_triangle(a, b, c);
      for (auto &p : meta_points) {
        auto final_p = naive_fragment_shader(p, height, width);
        image.set(final_p.pixel_pos[0], final_p.pixel_pos[1],
                  final_p.property.color_);
      }
    }
    {
      auto meta_points = shading.shading_triangle(b, c, d);
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