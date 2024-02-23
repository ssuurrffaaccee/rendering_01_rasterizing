#pragma once
#include <iostream>

#include "check.hpp"
#include "interval.hpp"
#include "mat.hpp"
#include "vec.hpp"
const Square<float, 2> NDC_tune_matrix{0.0f, -1.0f, 1.0f, 0.0f};
Vec<int, 2> NDC_to_Image(Vec2 ndc_pos, const Interval2D& image_range) {
  CHECK(image_range.get_min_x() == 0 && image_range.get_min_y() == 0)
  Vec2 image_size{float(image_range.get_max_x()),
                  float(image_range.get_max_y())};
  ndc_pos = mvdot(NDC_tune_matrix, ndc_pos);  // rotate counter-clock 90 degree;
  auto image_pos = (ndc_pos + Vec2{0.5f, 0.5f}) * image_size;
  return Vec<int, 2>{int(image_pos[0]), int(image_pos[1])};
}
