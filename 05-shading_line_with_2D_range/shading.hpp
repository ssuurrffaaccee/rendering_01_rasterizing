#pragma once
#include <tuple>

#include "check.hpp"
#include "interval.hpp"
#include "meta_point.hpp"
#include "vec.hpp"
class Shading {
 public:
  explicit Shading(const Interval2D& image_range) : image_range_{image_range} {}
  template <typename Property>
  std::vector<MetaPoint<Property>> shading_line(
      const MetaPoint<Property>& meta_point_a,
      const MetaPoint<Property>& meta_point_b) {
    if (meta_point_a.pixel_pos[0] == meta_point_b.pixel_pos[0]) {
      if (meta_point_a.pixel_pos[1] < meta_point_b.pixel_pos[1]) {
        return shading_vertical_line(meta_point_a, meta_point_b);
      } else {
        return shading_vertical_line(meta_point_b, meta_point_a);
      }
    }
    if (meta_point_a.pixel_pos[1] == meta_point_b.pixel_pos[1]) {
      if (meta_point_a.pixel_pos[0] < meta_point_b.pixel_pos[0]) {
        return shading_horizontal_line(meta_point_a, meta_point_b);
      } else {
        return shading_horizontal_line(meta_point_b, meta_point_a);
      }
    }
    auto [meta_point_p, meta_point_q, is_x] =
        sort_point(meta_point_a, meta_point_b);
    if (is_x) {
      int left_x = meta_point_p.pixel_pos[0];
      int right_x = meta_point_q.pixel_pos[0];
      std::vector<MetaPoint<Property>> meta_points;
      meta_points.reserve(right_x - left_x + 1);
      for (int x = left_x; x <= right_x; x++) {
        float ratio = float(x - left_x) / float(right_x - left_x);
        auto cur_meta_point = MetaPoint<Property>::mix_y(
            meta_point_p, meta_point_q, (1.0f - ratio), x);
        if (image_range_.is_in(cur_meta_point.pixel_pos[0],
                               cur_meta_point.pixel_pos[1])) {
          meta_points.push_back(cur_meta_point);
        }
      }
      return meta_points;
    } else {
      int down_y = meta_point_p.pixel_pos[1];
      int up_y = meta_point_q.pixel_pos[1];
      std::vector<MetaPoint<Property>> meta_points;
      meta_points.reserve(up_y - down_y + 1);
      for (int y = down_y; y <= up_y; y++) {
        float ratio = float(y - down_y) / float(up_y - down_y);
        auto cur_meta_point = MetaPoint<Property>::mix_x(
            meta_point_p, meta_point_q, (1.0f - ratio), y);
        if (image_range_.is_in(cur_meta_point.pixel_pos[0],
                               cur_meta_point.pixel_pos[1])) {
          meta_points.push_back(cur_meta_point);
        }
      }
      return meta_points;
    }
  }
  template <typename Property>
  std::vector<MetaPoint<Property>> shading_horizontal_line(
      const MetaPoint<Property>& meta_point_l,
      const MetaPoint<Property>& meta_point_r) {
    CHECK(meta_point_l.pixel_pos[1] == meta_point_r.pixel_pos[1]);
    if (image_range_.is_out_y(meta_point_l.pixel_pos[1])) {
      return std::vector<MetaPoint<Property>>{};
    }
    if (std::abs(meta_point_l.pixel_pos[0] - meta_point_r.pixel_pos[0]) <= 1) {
      if (image_range_.is_in(meta_point_l.pixel_pos[0],
                             meta_point_l.pixel_pos[1])) {
        return std::vector<MetaPoint<Property>>{meta_point_l};
      }
      return std::vector<MetaPoint<Property>>{};
    }
    CHECK(meta_point_l.pixel_pos[0] <= meta_point_r.pixel_pos[0]);
    int y = meta_point_l.pixel_pos[1];
    int left_x = meta_point_l.pixel_pos[0];
    int right_x = meta_point_r.pixel_pos[0];
    std::vector<MetaPoint<Property>> meta_points;
    meta_points.reserve(right_x - left_x + 1);
    if (meta_point_l.pixel_pos[0] == meta_point_r.pixel_pos[0]) {
      if (image_range_.is_out_x(meta_point_l.pixel_pos[0])) {
        return std::vector<MetaPoint<Property>>{};
      }
      meta_points.push_back(meta_point_l);
      return meta_points;
    }
    for (int x = left_x; x <= right_x; x++) {
      if (image_range_.is_out(x, y)) {
        continue;
      }
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
    if (image_range_.is_out_x(meta_point_down.pixel_pos[0])) {
      return std::vector<MetaPoint<Property>>{};
    }
    CHECK(meta_point_down.pixel_pos[1] <= meta_point_up.pixel_pos[1]);
    int x = meta_point_down.pixel_pos[0];
    int down_y = meta_point_down.pixel_pos[1];
    int up_y = meta_point_up.pixel_pos[1];
    std::vector<MetaPoint<Property>> meta_points;
    meta_points.reserve(up_y - down_y + 1);
    if (meta_point_down.pixel_pos[1] == meta_point_up.pixel_pos[1]) {
      if (image_range_.is_out_y(meta_point_down.pixel_pos[1])) {
        return std::vector<MetaPoint<Property>>{};
      }
      meta_points.push_back(meta_point_down);
      return meta_points;
    }
    for (int y = down_y; y <= up_y; y++) {
      if (image_range_.is_out(x, y)) {
        continue;
      }
      float ratio = float(y - down_y) / float(up_y - down_y);
      meta_points.push_back(MetaPoint<Property>::mix(
          meta_point_down, meta_point_up, (1.0f - ratio), x, y));
    }
    return meta_points;
  }

 private:
  // shading line helper
  template <typename Property>
  std::tuple<MetaPoint<Property>, MetaPoint<Property>, bool> sort_point(
      const MetaPoint<Property>& meta_point_a,
      const MetaPoint<Property>& meta_point_b) {
    int len_x = meta_point_a.pixel_pos[0] - meta_point_b.pixel_pos[0];
    int len_y = meta_point_a.pixel_pos[1] - meta_point_b.pixel_pos[1];
    bool is_x = std::abs(len_x) >= std::abs(len_y) ? true : false;
    int decision_len = is_x ? len_x : len_y;
    if (decision_len <= 0) {
      return std::tuple<MetaPoint<Property>, MetaPoint<Property>, bool>{
          meta_point_a, meta_point_b, is_x};
    } else {
      return std::tuple<MetaPoint<Property>, MetaPoint<Property>, bool>{
          meta_point_b, meta_point_a, is_x};
    }
  }
  // helper data
 private:
  Interval2D image_range_;
};