#pragma once
#include <tuple>

#include "check.hpp"
#include "meta_point.hpp"
#include "vec.hpp"
class Shading {
 public:
  explicit Shading() {}
  template <typename Property>
  std::vector<MetaPoint<Property>> shading_horizontal_line(
      const MetaPoint<Property>& meta_point_l,
      const MetaPoint<Property>& meta_point_r) {
    CHECK(meta_point_l.pixel_pos[1] == meta_point_r.pixel_pos[1]);
    CHECK(meta_point_l.pixel_pos[0] <= meta_point_r.pixel_pos[0]);
    int y = meta_point_l.pixel_pos[1];
    int left_x = meta_point_l.pixel_pos[0];
    int right_x = meta_point_r.pixel_pos[0];
    std::vector<MetaPoint<Property>> meta_points;
    meta_points.reserve(right_x - left_x + 1);
    if (meta_point_l.pixel_pos[0] == meta_point_r.pixel_pos[0]) {
      meta_points.push_back(meta_point_l);
      return meta_points;
    }
    for (int x = left_x; x <= right_x; x++) {
      float ratio = float(x - left_x) / float(right_x - left_x);
      meta_points.push_back(MetaPoint<Property>::mix(meta_point_l, meta_point_r,
                                                     (1.0f - ratio), x, y));
    }
    return meta_points;
  }
  template <typename Property>
  std::vector<MetaPoint<Property>> shading_vertical_line(
      const MetaPoint<Property>& meta_point_down,
      const MetaPoint<Property>& meta_point_up) {
    CHECK(meta_point_down.pixel_pos[0] == meta_point_up.pixel_pos[0]);
    CHECK(meta_point_down.pixel_pos[1] <= meta_point_up.pixel_pos[1]);
    int x = meta_point_down.pixel_pos[0];
    int down_y = meta_point_down.pixel_pos[1];
    int up_y = meta_point_up.pixel_pos[1];
    std::vector<MetaPoint<Property>> meta_points;
    meta_points.reserve(up_y - down_y + 1);
    if (meta_point_down.pixel_pos[1] == meta_point_up.pixel_pos[1]) {
      meta_points.push_back(meta_point_down);
      return meta_points;
    }
    for (int y = down_y; y <= up_y; y++) {
      float ratio = float(y - down_y) / float(up_y - down_y);
      meta_points.push_back(MetaPoint<Property>::mix(
          meta_point_down, meta_point_up, (1.0f - ratio), x, y));
    }
    return meta_points;
  }
};