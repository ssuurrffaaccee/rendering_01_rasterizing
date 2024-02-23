#include <iostream>

#include "image.hpp"
#include "shading.hpp"
int main() {
  try {
    int height = 1000;
    int width = 1000;
    Image image{size_t(width), size_t(height)};
    Shading shading;
    {
      MetaPoint<ColorProperty> down{{500, 0}, Color{1.0f, 0.0f, 0.0f}};
      MetaPoint<ColorProperty> up{{500, 999}, Color{0.0f, 1.0f, 0.0f}};
      auto meta_points = shading.shading_vertical_line(down, up);
      for (auto &p : meta_points) {
        image.set(p.pixel_pos[0], p.pixel_pos[1], p.property.color_);
      }
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}