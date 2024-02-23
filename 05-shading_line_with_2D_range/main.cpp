#include <iostream>

#include "image.hpp"
#include "shading.hpp"
void draw(const std::vector<MetaPoint<Color>>& meta_points, Image& image) {
  for (auto& p : meta_points) {
    image.set(p.pixel_pos[0], p.pixel_pos[1], p.property);
  }
}
std::vector<std::pair<MetaPoint<Color>, MetaPoint<Color>>> generate_lines(
    float len, int num, int center_x, int center_y) {
  std::vector<std::pair<MetaPoint<Color>, MetaPoint<Color>>> lines;
  float degree = 360.0f / num;
  for (int i = 0; i < num; i++) {
    float x = len * std::cos(i * degree * 3.14159f / 180.0f);
    float y = len * std::sin(i * degree * 3.14159f / 180.0f);
    lines.push_back(std::make_pair(
        MetaPoint<Color>{{center_x, center_y}, Color{1.0f, 0.0, 0.0f}},
        MetaPoint<Color>{{center_x + int(x), center_y + int(y)},
                         Color{0.0f, 1.0f, 0.0f}}));
  }
  return lines;
}
int main() {
  try {
    size_t height = 1000;
    size_t width = 1000;
    Image image{width, height};
    Shading shading{image.get_range()};
    int centor_x = int(width / 2);
    int centor_y = int(height / 2);
    float radius = 1000.0f;
    int line_num = 10;
    auto lines = generate_lines(radius, line_num, centor_x, centor_y);
    for (auto& [a, b] : lines) {
      auto meta_points = shading.shading_line(a, b);
      draw(meta_points, image);
    }
    image.dump("xxxx");
  } catch (MyExceptoin& e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}