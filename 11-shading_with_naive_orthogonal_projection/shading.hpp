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
  std::vector<MetaPoint<Property>> shading_triangle(
      const MetaPoint<Property>& meta_point_0,
      const MetaPoint<Property>& meta_point_1,
      const MetaPoint<Property>& meta_point_2) {
    if (image_range_.is_out(meta_point_0.pixel_pos[0],
                            meta_point_0.pixel_pos[1]) &&
        image_range_.is_out(meta_point_1.pixel_pos[0],
                            meta_point_1.pixel_pos[1]) &&
        image_range_.is_out(meta_point_2.pixel_pos[0],
                            meta_point_2.pixel_pos[1])) {
      return std::vector<MetaPoint<Property>>{};
    }
    if (meta_point_0.pixel_pos[0] == meta_point_1.pixel_pos[0] &&
        meta_point_0.pixel_pos[0] == meta_point_2.pixel_pos[0]) {
      std::vector<MetaPoint<Property>> meta_points;
      auto meta_points_01 = shading_line(meta_point_0, meta_point_1);
      meta_points.insert(meta_points.end(), meta_points_01.begin(),
                         meta_points_01.end());
      auto meta_points_12 = shading_line(meta_point_1, meta_point_2);
      meta_points.insert(meta_points.end(), meta_points_12.begin(),
                         meta_points_12.end());
      auto meta_points_10 = shading_line(meta_point_1, meta_point_0);
      meta_points.insert(meta_points.end(), meta_points_10.begin(),
                         meta_points_10.end());
      return meta_points;
    }
    if (meta_point_0.pixel_pos[0] == meta_point_1.pixel_pos[0] &&
        meta_point_0.pixel_pos[1] == meta_point_1.pixel_pos[1]) {
      return shading_line(meta_point_0, meta_point_2);
    }
    if (meta_point_0.pixel_pos[0] == meta_point_2.pixel_pos[0] &&
        meta_point_0.pixel_pos[1] == meta_point_2.pixel_pos[1]) {
      return shading_line(meta_point_0, meta_point_1);
    }
    if (meta_point_1.pixel_pos[0] == meta_point_2.pixel_pos[0] &&
        meta_point_1.pixel_pos[1] == meta_point_2.pixel_pos[1]) {
      return shading_line(meta_point_0, meta_point_1);
    }
    auto [meta_point_a, meta_point_b, meta_point_c] =
        sort_triangle_point(meta_point_0, meta_point_1, meta_point_2);
    std::vector<MetaPoint<Property>> left_sequence =
        get_border_sequence(meta_point_a, meta_point_c);
    auto [right_sequence, max_dis_index] =
        get_border_sequence(meta_point_a, meta_point_b, meta_point_c);
    CHECK(left_sequence.size() == right_sequence.size())
    bool is_ac_left = true;
    for (size_t i = 0; i < left_sequence.size(); i++) {
      if (left_sequence[i].pixel_pos[0] == right_sequence[i].pixel_pos[0]) {
        continue;
      }
      is_ac_left =
          left_sequence[i].pixel_pos[0] < right_sequence[i].pixel_pos[0]
              ? true
              : false;
      break;
    }
    if (!is_ac_left) {
      std::swap(left_sequence, right_sequence);
    }
    std::vector<MetaPoint<Property>> meta_points;
    for (int i = 0; i < left_sequence.size(); i++) {
      auto line_meta_points =
          shading_horizontal_line(left_sequence[i], right_sequence[i]);
      meta_points.insert(meta_points.end(), line_meta_points.begin(),
                         line_meta_points.end());
    }
    return meta_points;
  }
  template <typename Property>
  std::vector<MetaPoint<Property>> shading_triangle_frame(
      const MetaPoint<Property>& meta_point_0,
      const MetaPoint<Property>& meta_point_1,
      const MetaPoint<Property>& meta_point_2) {
    auto meta_points_01 = shading_line(meta_point_0, meta_point_1);
    auto meta_points_12 = shading_line(meta_point_1, meta_point_2);
    auto meta_points_20 = shading_line(meta_point_2, meta_point_0);
    std::vector<MetaPoint<Property>> meta_points;
    for (auto& p : meta_points_01) {
      if (image_range_.is_in(p.pixel_pos[0], p.pixel_pos[1])) {
        meta_points.push_back(p);
      }
    }
    for (auto& p : meta_points_12) {
      if (image_range_.is_in(p.pixel_pos[0], p.pixel_pos[1])) {
        meta_points.push_back(p);
      }
    }
    for (auto& p : meta_points_20) {
      if (image_range_.is_in(p.pixel_pos[0], p.pixel_pos[1])) {
        meta_points.push_back(p);
      }
    }
    return meta_points;
  }
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
  // shading triangle helper
  template <typename Property>
  std::vector<MetaPoint<Property>> get_border_sequence(
      const MetaPoint<Property>& meta_point_a,
      const MetaPoint<Property>& meta_point_c) {
    int down_y = meta_point_c.pixel_pos[1];
    int up_y = meta_point_a.pixel_pos[1];
    CHECK(down_y <= up_y)
    std::vector<MetaPoint<Property>> meta_points;
    meta_points.reserve(up_y - down_y + 1);
    for (int y = down_y; y <= up_y; y++) {
      float ratio = float(y - down_y) / float(up_y - down_y);
      meta_points.push_back(
          MetaPoint<Property>::mix_x(meta_point_a, meta_point_c, ratio, y));
    }
    return meta_points;
  }
  template <typename Property>
  std::pair<std::vector<MetaPoint<Property>>, int> get_border_sequence(
      const MetaPoint<Property>& meta_point_a,
      const MetaPoint<Property>& meta_point_b,
      const MetaPoint<Property>& meta_point_c) {
    if (meta_point_a.pixel_pos[1] == meta_point_b.pixel_pos[1]) {
      return std::make_pair(get_border_sequence(meta_point_b, meta_point_c), 0);
    } else if (meta_point_b.pixel_pos[1] == meta_point_c.pixel_pos[1]) {
      auto temp = get_border_sequence(meta_point_a, meta_point_b);
      return std::make_pair(std::move(temp), temp.size());
    } else {
      auto ab_sequence = get_border_sequence(meta_point_a, meta_point_b);
      auto bc_sequence = get_border_sequence(meta_point_b, meta_point_c);
      std::vector<MetaPoint<Property>> meta_points;
      meta_points.reserve(ab_sequence.size() + bc_sequence.size());
      meta_points.insert(meta_points.end(), bc_sequence.begin(),
                         bc_sequence.end());
      meta_points.insert(meta_points.end(), ab_sequence.begin() + 1,
                         ab_sequence.end());
      return std::make_pair(std::move(meta_points), int(bc_sequence.size()));
    }
  }
  template <typename Property>
  std::tuple<MetaPoint<Property>, MetaPoint<Property>, MetaPoint<Property>>
  sort_triangle_point(const MetaPoint<Property>& meta_point_0,
                      const MetaPoint<Property>& meta_point_1,
                      const MetaPoint<Property>& meta_point_2) {
    std::vector<int> ys{meta_point_0.pixel_pos[1], meta_point_1.pixel_pos[1],
                        meta_point_2.pixel_pos[1]};
    int max_index = 0;
    if (meta_point_1.pixel_pos[1] > ys[max_index]) {
      max_index = 1;
    }
    if (meta_point_2.pixel_pos[1] > ys[max_index]) {
      max_index = 2;
    }
    if (max_index == 0) {
      if (meta_point_1.pixel_pos[1] > meta_point_2.pixel_pos[1]) {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_0, meta_point_1,
                                               meta_point_2};
      } else {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_0, meta_point_2,
                                               meta_point_1};
      }
    } else if (max_index == 1) {
      if (meta_point_0.pixel_pos[1] > meta_point_2.pixel_pos[1]) {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_1, meta_point_0,
                                               meta_point_2};
      } else {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_1, meta_point_2,
                                               meta_point_0};
      }
    } else {
      if (meta_point_0.pixel_pos[1] > meta_point_1.pixel_pos[1]) {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_2, meta_point_0,
                                               meta_point_1};
      } else {
        return std::tuple<MetaPoint<Property>, MetaPoint<Property>,
                          MetaPoint<Property>>{meta_point_2, meta_point_1,
                                               meta_point_0};
      }
    }
  }
  // helper data
 private:
  Interval2D image_range_;
};