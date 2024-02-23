#include <iostream>

#include "image.hpp"
#include "ray.hpp"
int main() {
  Vec3 origin{0.0f, 0.0f, 0.0f};
  Vec3 forward{0.0f, 0.0f, .2f};

  size_t height = 1000;
  Vec3 height_vec = {0.0f, 1.0f, 0.0f};
  size_t width = 1000;
  Vec3 width_vec = {1.0f, 0.0f, 0.0f};
  Image image{width, height};
  Vec3 offset = {-0.5f, -0.5f, 0.0f};
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      Vec3 pixel_pos = (x / float(width)) * width_vec +
                       (y / float(height)) * height_vec + forward + offset;
      Ray ray = Ray{origin,normalize(pixel_pos-origin)};
      Color color = abs(ray.direction_);
      image.set(x, y, color);
    }
  }
  image.dump("xxxx");
  return 0;
}