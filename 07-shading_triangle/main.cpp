#include <iostream>

#include "image.hpp"
#include "shading.hpp"
int main() {
  try {
    size_t height = 1000;
    size_t width = 1000;
    Image image{width, height};
    Shading shading{image.get_range()};
    // triangle 0
    MetaPoint<ColorProperty> a{{10, 10}, {1.0f, 0.0f, 0.0f}};
    MetaPoint<ColorProperty> b{{157, 10}, {0.0f, 1.0f, 0.0f}};
    MetaPoint<ColorProperty> c{{10, 157}, {0.0f, 0.0f, 1.0f}};
    // triangle 1
    MetaPoint<ColorProperty> e{{78, 10}, {0.0f, 0.0f, 1.0f}};
    MetaPoint<ColorProperty> f{{157, 90}, {0.0f, 1.0f, 0.0f}};
    MetaPoint<ColorProperty> g{{10, 157}, {1.0f, 0.0f, 0.0f}};

    int x = 0;
    for (int dx = 0; dx < 1000; dx += 170) {
      for (int dy = 0; dy < 1000; dy += 170) {
        x = 1 - x;
        if (x == 0) {
          Vec<int, 2> offset{dx, dy};
          auto new_a = a;
          new_a.pixel_pos += offset;
          auto new_b = b;
          new_b.pixel_pos += offset;
          auto new_c = c;
          new_c.pixel_pos += offset;
          auto meta_points = shading.shading_triangle(new_a, new_b, new_c);
          for (auto &p : meta_points) {
            image.set(p.pixel_pos[0], p.pixel_pos[1], p.property.color_);
          }
        } else {
          Vec<int, 2> offset{dx, dy};
          auto new_e = e;
          new_e.pixel_pos += offset;
          auto new_f = f;
          new_f.pixel_pos += offset;
          auto new_g = g;
          new_g.pixel_pos += offset;
          auto meta_points = shading.shading_triangle(new_e, new_f, new_g);
          for (auto &p : meta_points) {
            image.set(p.pixel_pos[0], p.pixel_pos[1], p.property.color_);
          }
        }
      }
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}