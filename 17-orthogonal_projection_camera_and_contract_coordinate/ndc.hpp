#pragma once
#include <iostream>

#include "check.hpp"
#include "interval.hpp"
#include "mat.hpp"
#include "vec.hpp"
Vec<int, 2> NDC_to_Image(Vec2 ndc_pos, const Interval2D& image_range) {
  CHECK(image_range.get_min_x() == 0 && image_range.get_min_y() == 0)
  Vec2 image_size{float(image_range.get_max_x()),
                  float(image_range.get_max_y())};
  auto image_pos = (ndc_pos + Vec2{0.5f, 0.5f}) * image_size;
  image_pos[1] = image_size[1] - image_pos[1];  // flip y
  return Vec<int, 2>{int(image_pos[0]), int(image_pos[1])};
}
