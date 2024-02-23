#include <iostream>

#include "image.hpp"
int main() {
  size_t height = 1000;
  size_t width = 1000;
  Image image{width, height};
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      image.set(x, y, Color{my_random(), my_random(), my_random()});
    }
  }
  image.dump("xxxx");
  return 0;
}