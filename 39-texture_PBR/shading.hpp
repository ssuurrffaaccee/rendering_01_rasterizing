#pragma once
#include <tuple>

#include "check.hpp"
#include "interval.hpp"
#include "shader.hpp"
#include "vec.hpp"
class Shading {
 public:
  static std::vector<FragmentStageData> shading_triangle(
      const FragmentStageData &frag_0, const FragmentStageData &frag_1,
      const FragmentStageData &frag_2) {
    
    if (frag_0.first[0] == frag_1.first[0] &&
        frag_0.first[0] == frag_2.first[0] &&
        frag_0.first[1] == frag_1.first[1] &&
        frag_0.first[1] == frag_2.first[1]) {
      return std::vector<FragmentStageData>{frag_0};
    }
    if (frag_0.first[0] == frag_1.first[0] &&
            frag_0.first[0] == frag_2.first[0] ||
        frag_0.first[1] == frag_1.first[1] &&
            frag_0.first[1] == frag_2.first[1]) {
      std::vector<FragmentStageData> meta_points;
      auto meta_points_01 = shading_line(frag_0, frag_1);
      meta_points.insert(meta_points.end(), meta_points_01.begin(),
                         meta_points_01.end());
      auto meta_points_12 = shading_line(frag_1, frag_2);
      meta_points.insert(meta_points.end(), meta_points_12.begin(),
                         meta_points_12.end());
      auto meta_points_10 = shading_line(frag_1, frag_0);
      meta_points.insert(meta_points.end(), meta_points_10.begin(),
                         meta_points_10.end());
      return meta_points;
    }
    if (frag_0.first[0] == frag_1.first[0] &&
        frag_0.first[1] == frag_1.first[1]) {
      return shading_line(frag_0, frag_2);
    }
    if (frag_0.first[0] == frag_2.first[0] &&
        frag_0.first[1] == frag_2.first[1]) {
      return shading_line(frag_0, frag_1);
    }
    if (frag_1.first[0] == frag_2.first[0] &&
        frag_1.first[1] == frag_2.first[1]) {
      return shading_line(frag_0, frag_1);
    }
    auto [frag_a, frag_b, frag_c] = sort_triangle_point(frag_0, frag_1, frag_2);
    std::vector<FragmentStageData> left_sequence =
        get_border_sequence(frag_a, frag_c);
    auto [right_sequence, max_dis_index] =
        get_border_sequence(frag_a, frag_b, frag_c);
    CHECK(left_sequence.size() == right_sequence.size())
    bool is_ac_left = true;
    for (size_t i = 0; i < left_sequence.size(); i++) {
      if (left_sequence[i].first[0] == right_sequence[i].first[0]) {
        continue;
      }
      is_ac_left =
          left_sequence[i].first[0] < right_sequence[i].first[0] ? true : false;
      break;
    }
    if (!is_ac_left) {
      std::swap(left_sequence, right_sequence);
    }
    std::vector<FragmentStageData> meta_points;
    for (int i = 0; i < left_sequence.size(); i++) {
      auto line_meta_points =
          shading_horizontal_line(left_sequence[i], right_sequence[i]);
      meta_points.insert(meta_points.end(), line_meta_points.begin(),
                         line_meta_points.end());
    }
    return meta_points;
  }
  static std::vector<FragmentStageData> shading_triangle_frame(
      const FragmentStageData &frag_0, const FragmentStageData &frag_1,
      const FragmentStageData &frag_2) {
    if (frag_0.first[0] == frag_1.first[0] &&
        frag_0.first[0] == frag_2.first[0] &&
        frag_0.first[1] == frag_1.first[1] &&
        frag_0.first[1] == frag_2.first[1]) {
      return std::vector<FragmentStageData>{frag_0};
    }
    auto meta_points_01 = shading_line(frag_0, frag_1);
    auto meta_points_12 = shading_line(frag_1, frag_2);
    auto meta_points_20 = shading_line(frag_2, frag_0);
    std::vector<FragmentStageData> meta_points;
    meta_points.insert(meta_points.end(), meta_points_01.begin(),
                       meta_points_01.end());
    meta_points.insert(meta_points.end(), meta_points_12.begin(),
                       meta_points_12.end());
    meta_points.insert(meta_points.end(), meta_points_20.begin(),
                       meta_points_20.end());
    return meta_points;
  }
  static std::vector<FragmentStageData> shading_line(
      const FragmentStageData &frag_a, const FragmentStageData &frag_b) {
    if (frag_a.first[0] == frag_b.first[0]) {
      if (frag_a.first[1] < frag_b.first[1]) {
        return shading_vertical_line(frag_a, frag_b);
      } else {
        return shading_vertical_line(frag_b, frag_a);
      }
    }
    if (frag_a.first[1] == frag_b.first[1]) {
      if (frag_a.first[0] < frag_b.first[0]) {
        return shading_horizontal_line(frag_a, frag_b);
      } else {
        return shading_horizontal_line(frag_b, frag_a);
      }
    }
    auto [meta_point_p, meta_point_q, is_x] = sort_point(frag_a, frag_b);
    if (is_x) {
      int left_x = meta_point_p.first[0];
      int right_x = meta_point_q.first[0];
      std::vector<FragmentStageData> meta_points;
      meta_points.reserve(right_x - left_x + 1);
      for (int x = left_x; x <= right_x; x++) {
        float ratio = float(x - left_x) / float(right_x - left_x);
        auto cur_meta_point =
            ::mix_y(meta_point_p, meta_point_q, (1.0f - ratio), x);
        meta_points.push_back(cur_meta_point);
      }
      return meta_points;
    } else {
      int down_y = meta_point_p.first[1];
      int up_y = meta_point_q.first[1];
      std::vector<FragmentStageData> meta_points;
      meta_points.reserve(up_y - down_y + 1);
      for (int y = down_y; y <= up_y; y++) {
        float ratio = float(y - down_y) / float(up_y - down_y);
        auto cur_meta_point =
            ::mix_x(meta_point_p, meta_point_q, (1.0f - ratio), y);
        meta_points.push_back(cur_meta_point);
      }
      return meta_points;
    }
  }
  static std::vector<FragmentStageData> shading_horizontal_line(
      const FragmentStageData &meta_point_l,
      const FragmentStageData &meta_point_r) {
    CHECK(meta_point_l.first[1] == meta_point_r.first[1]);
    if (std::abs(meta_point_l.first[0] - meta_point_r.first[0]) <= 1) {
      return std::vector<FragmentStageData>{meta_point_l};
    }
    CHECK(meta_point_l.first[0] <= meta_point_r.first[0]);
    int y = meta_point_l.first[1];
    int left_x = meta_point_l.first[0];
    int right_x = meta_point_r.first[0];
    std::vector<FragmentStageData> meta_points;
    meta_points.reserve(right_x - left_x + 1);
    if (meta_point_l.first[0] == meta_point_r.first[0]) {
      meta_points.push_back(meta_point_l);
      return meta_points;
    }
    for (int x = left_x; x <= right_x; x++) {
      float ratio = float(x - left_x) / float(right_x - left_x);
      meta_points.push_back(
          ::mix(meta_point_l, meta_point_r, (1.0f - ratio), x, y));
    }
    return meta_points;
  }
  static std::vector<FragmentStageData> shading_vertical_line(
      const FragmentStageData &meta_point_down,
      const FragmentStageData &meta_point_up) {
    CHECK(meta_point_down.first[0] == meta_point_up.first[0]);
    CHECK(meta_point_down.first[1] <= meta_point_up.first[1]);
    int x = meta_point_down.first[0];
    int down_y = meta_point_down.first[1];
    int up_y = meta_point_up.first[1];
    std::vector<FragmentStageData> meta_points;
    meta_points.reserve(up_y - down_y + 1);
    if (meta_point_down.first[1] == meta_point_up.first[1]) {
      meta_points.push_back(meta_point_down);
      return meta_points;
    }
    for (int y = down_y; y <= up_y; y++) {
      float ratio = float(y - down_y) / float(up_y - down_y);
      meta_points.push_back(
          ::mix(meta_point_down, meta_point_up, (1.0f - ratio), x, y));
    }
    return meta_points;
  }

 private:
  // shading line helper
  static std::tuple<FragmentStageData, FragmentStageData, bool> sort_point(
      const FragmentStageData &frag_a, const FragmentStageData &frag_b) {
    int len_x = frag_a.first[0] - frag_b.first[0];
    int len_y = frag_a.first[1] - frag_b.first[1];
    bool is_x = std::abs(len_x) >= std::abs(len_y) ? true : false;
    int decision_len = is_x ? len_x : len_y;
    if (decision_len <= 0) {
      return std::tuple<FragmentStageData, FragmentStageData, bool>{
          frag_a, frag_b, is_x};
    } else {
      return std::tuple<FragmentStageData, FragmentStageData, bool>{
          frag_b, frag_a, is_x};
    }
  }
  // shading triangle helper
  static std::vector<FragmentStageData> get_border_sequence(
      const FragmentStageData &frag_a, const FragmentStageData &frag_c) {
    int down_y = frag_c.first[1];
    int up_y = frag_a.first[1];
    CHECK(down_y <= up_y)
    std::vector<FragmentStageData> meta_points;
    meta_points.reserve(up_y - down_y + 1);
    for (int y = down_y; y <= up_y; y++) {
      float ratio = float(y - down_y) / float(up_y - down_y);
      meta_points.push_back(::mix_x(frag_a, frag_c, ratio, y));
    }
    return meta_points;
  }
  static std::pair<std::vector<FragmentStageData>, int> get_border_sequence(
      const FragmentStageData &frag_a, const FragmentStageData &frag_b,
      const FragmentStageData &frag_c) {
    if (frag_a.first[1] == frag_b.first[1]) {
      return std::make_pair(get_border_sequence(frag_b, frag_c), 0);
    } else if (frag_b.first[1] == frag_c.first[1]) {
      auto temp = get_border_sequence(frag_a, frag_b);
      return std::make_pair(std::move(temp), temp.size());
    } else {
      auto ab_sequence = get_border_sequence(frag_a, frag_b);
      auto bc_sequence = get_border_sequence(frag_b, frag_c);
      std::vector<FragmentStageData> meta_points;
      meta_points.reserve(ab_sequence.size() + bc_sequence.size());
      meta_points.insert(meta_points.end(), bc_sequence.begin(),
                         bc_sequence.end());
      meta_points.insert(meta_points.end(), ab_sequence.begin() + 1,
                         ab_sequence.end());
      return std::make_pair(std::move(meta_points), int(bc_sequence.size()));
    }
  }
  static std::tuple<FragmentStageData, FragmentStageData, FragmentStageData>
  sort_triangle_point(const FragmentStageData &frag_0,
                      const FragmentStageData &frag_1,
                      const FragmentStageData &frag_2) {
    std::vector<int> ys{frag_0.first[1], frag_1.first[1], frag_2.first[1]};
    int max_index = 0;
    if (frag_1.first[1] > ys[max_index]) {
      max_index = 1;
    }
    if (frag_2.first[1] > ys[max_index]) {
      max_index = 2;
    }
    if (max_index == 0) {
      if (frag_1.first[1] > frag_2.first[1]) {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_0, frag_1, frag_2};
      } else {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_0, frag_2, frag_1};
      }
    } else if (max_index == 1) {
      if (frag_0.first[1] > frag_2.first[1]) {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_1, frag_0, frag_2};
      } else {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_1, frag_2, frag_0};
      }
    } else {
      if (frag_0.first[1] > frag_1.first[1]) {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_2, frag_0, frag_1};
      } else {
        return std::tuple<FragmentStageData, FragmentStageData,
                          FragmentStageData>{frag_2, frag_1, frag_0};
      }
    }
  }
};