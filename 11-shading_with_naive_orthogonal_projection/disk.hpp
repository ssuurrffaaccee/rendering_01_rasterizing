#include <unordered_map>
#include <vector>

#include "vec.hpp"
class DiskGenerator {
 public:
  explicit DiskGenerator(size_t level) : level_{level} {
    is_finished_ = false;
    // CHECK(level_  > 0)
    points_[0] = Vec2{0.0f, 0.0f};
    points_[1] = Vec2{0.5f, 0.0f};
    points_[2] = Vec2{0.0f, 0.5f};
    points_[3] = Vec2{-0.5f, 0.0f};
    points_[4] = Vec2{0.0f, -0.5f};
    next_point_id_ = 5;

    triangles_.resize(4);
    triangles_[0] = std::tuple<int, int, int>{0, 1, 2};
    triangles_[1] = std::tuple<int, int, int>{0, 2, 3};
    triangles_[2] = std::tuple<int, int, int>{0, 3, 4};
    triangles_[3] = std::tuple<int, int, int>{0, 4, 1};
  }
  int get_next_point_id() { return next_point_id_++; }
  std::pair<std::vector<Vec2>, std::vector<std::tuple<int, int, int>>> get() {
    if (!is_finished_) {
      for (size_t l = 0; l < level_; l++) {
        std::vector<std::tuple<int, int, int>> new_triangles;
        new_triangles.reserve(triangles_.size() * 2);
        for (auto& [a_idx, b_idx, c_idx] : triangles_) {
          int mid_point_id = get_next_point_id();
          auto mid_point = normalize(points_[b_idx] + points_[c_idx]) * 0.5f;
          points_[mid_point_id] = mid_point;
          new_triangles.push_back(
              std::tuple<int, int, int>{a_idx, b_idx, mid_point_id});
          new_triangles.push_back(
              std::tuple<int, int, int>{a_idx, mid_point_id, c_idx});
        }
        std::swap(new_triangles, triangles_);
      }
      is_finished_ = true;
    }
    std::vector<Vec2> points;
    for (int i = 0; i < points_.size(); i++) {
      points.push_back(points_[i]);
    }
    return std::make_pair(std::move(points), triangles_);
  }
  bool is_finished_;
  size_t level_;
  int next_point_id_;
  std::vector<std::tuple<int, int, int>> triangles_;
  std::unordered_map<int, Vec2> points_;
};