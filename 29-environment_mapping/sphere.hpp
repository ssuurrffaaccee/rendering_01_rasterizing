#include <unordered_map>
#include <vector>

#include "model.hpp"
#include "vec.hpp"
class SphereGenerator0 {
 public:
  explicit SphereGenerator0(size_t level) : level_{level} {
    is_finished_ = false;
    // CHECK(level_  > 0)
    points_[0] = Vec3{0.5f, 0.0f, 0.0f};
    points_[1] = Vec3{0.0f, 0.5f, 0.0f};
    points_[2] = Vec3{0.0f, 0.0f, 0.5f};
    points_[3] = Vec3{-0.5f, 0.0f, 0.0f};
    points_[4] = Vec3{0.0f, -0.5f, 0.0f};
    points_[5] = Vec3{0.0f, 0.0f, -0.5f};
    triangles_.resize(8);
    triangles_[0] = std::tuple<int, int, int>{2, 0, 1};
    triangles_[1] = std::tuple<int, int, int>{2, 1, 3};
    triangles_[2] = std::tuple<int, int, int>{2, 3, 4};
    triangles_[3] = std::tuple<int, int, int>{2, 4, 0};
    triangles_[4] = std::tuple<int, int, int>{5, 1, 0};
    triangles_[5] = std::tuple<int, int, int>{5, 3, 1};
    triangles_[6] = std::tuple<int, int, int>{5, 4, 3};
    triangles_[7] = std::tuple<int, int, int>{5, 0, 4};
    next_point_id_ = 6;
  }
  int get_next_point_id() { return next_point_id_++; }
  Model get() {
    if (!is_finished_) {
      for (size_t l = 0; l < level_; l++) {
        std::vector<std::tuple<int, int, int>> new_triangles;
        new_triangles.reserve(triangles_.size() * 3);
        for (auto& [a_idx, b_idx, c_idx] : triangles_) {
          int mid_point_id = get_next_point_id();
          auto mid_point =
              normalize(points_[a_idx] + points_[b_idx] + points_[c_idx]) * 0.5;
          points_[mid_point_id] = mid_point;
          new_triangles.push_back(
              std::tuple<int, int, int>{a_idx, b_idx, mid_point_id});
          new_triangles.push_back(
              std::tuple<int, int, int>{b_idx, c_idx, mid_point_id});
          new_triangles.push_back(
              std::tuple<int, int, int>{c_idx, a_idx, mid_point_id});
        }
        std::swap(new_triangles, triangles_);
      }
      is_finished_ = true;
    }
    Model model;
    model.triangles_ = triangles_;
    model.points_.reserve(points_.size());
    for (int i = 0; i < points_.size(); i++) {
      model.points_.push_back(points_[i]);
    }
    return model;
  }
  bool is_finished_;
  size_t level_;
  int next_point_id_;
  std::vector<std::tuple<int, int, int>> triangles_;
  std::unordered_map<int, Vec3> points_;
};

class SphereGenerator1 {
 public:
  explicit SphereGenerator1(size_t level) : level_{level} {
    is_finished_ = false;
    // CHECK(level_  > 0)
    points_[0] = Vec3{0.5f, 0.0f, 0.0f};
    points_[1] = Vec3{0.0f, 0.5f, 0.0f};
    points_[2] = Vec3{0.0f, 0.0f, 0.5f};
    points_[3] = Vec3{-0.5f, 0.0f, 0.0f};
    points_[4] = Vec3{0.0f, -0.5f, 0.0f};
    points_[5] = Vec3{0.0f, 0.0f, -0.5f};
    triangles_.resize(8);
    triangles_[0] = std::tuple<int, int, int>{2, 0, 1};
    triangles_[1] = std::tuple<int, int, int>{2, 1, 3};
    triangles_[2] = std::tuple<int, int, int>{2, 3, 4};
    triangles_[3] = std::tuple<int, int, int>{2, 4, 0};
    triangles_[4] = std::tuple<int, int, int>{5, 1, 0};
    triangles_[5] = std::tuple<int, int, int>{5, 3, 1};
    triangles_[6] = std::tuple<int, int, int>{5, 4, 3};
    triangles_[7] = std::tuple<int, int, int>{5, 0, 4};
    next_point_id_ = 6;
  }
  int get_next_point_id() { return next_point_id_++; }
  Model get() {
    if (!is_finished_) {
      for (size_t l = 0; l < level_; l++) {
        std::vector<std::tuple<int, int, int>> new_triangles;
        new_triangles.reserve(triangles_.size() * 3);
        for (auto& [a_idx, b_idx, c_idx] : triangles_) {
          int ab_mid_point_id = get_next_point_id();
          auto ab_mid_point = normalize(points_[a_idx] + points_[b_idx]) * 0.5;
          points_[ab_mid_point_id] = ab_mid_point;

          int bc_mid_point_id = get_next_point_id();
          auto bc_mid_point = normalize(points_[b_idx] + points_[c_idx]) * 0.5;
          points_[bc_mid_point_id] = bc_mid_point;

          int ca_mid_point_id = get_next_point_id();
          auto ca_mid_point = normalize(points_[c_idx] + points_[a_idx]) * 0.5;
          points_[ca_mid_point_id] = ca_mid_point;

          new_triangles.push_back(std::tuple<int, int, int>{
              a_idx, ab_mid_point_id, ca_mid_point_id});
          new_triangles.push_back(std::tuple<int, int, int>{
              ab_mid_point_id, b_idx, bc_mid_point_id});
          new_triangles.push_back(std::tuple<int, int, int>{
              bc_mid_point_id, c_idx, ca_mid_point_id});
          new_triangles.push_back(std::tuple<int, int, int>{
              ab_mid_point_id, bc_mid_point_id, ca_mid_point_id});
        }
        std::swap(new_triangles, triangles_);
      }
      is_finished_ = true;
    }
    Model model;
    model.triangles_ = triangles_;
    model.points_.reserve(points_.size());
    for (int i = 0; i < points_.size(); i++) {
      model.points_.push_back(points_[i]);
    }
    return model;
  }
  bool is_finished_;
  size_t level_;
  int next_point_id_;
  std::vector<std::tuple<int, int, int>> triangles_;
  std::unordered_map<int, Vec3> points_;
};
class Cone {
 public:
  explicit Cone(size_t level) : level_{level} {
    is_finished_ = false;
    // CHECK(level_  > 0)
    points_[0] = Vec3{0.5f, 0.0f, 0.0f};
    points_[1] = Vec3{0.0f, 0.0f, -0.5f};
    points_[2] = Vec3{-0.5f, 0.0f, 0.0f};
    points_[3] = Vec3{0.0f, 0.0f, 0.5f};
    points_[4] = Vec3{0.0f, 0.5f, 0.0f};  // center
    points_[5] = Vec3{0.0f, 0.0f, 0.0f};  // center
    triangles_pairs_.resize(4);
    triangles_pairs_[0] = std::tuple<int, int, int, int>{4, 5, 0, 1};
    triangles_pairs_[1] = std::tuple<int, int, int, int>{4, 5, 1, 2};
    triangles_pairs_[2] = std::tuple<int, int, int, int>{4, 5, 2, 3};
    triangles_pairs_[3] = std::tuple<int, int, int, int>{4, 5, 3, 0};
    next_point_id_ = 6;
  }
  int get_next_point_id() { return next_point_id_++; }
  Model get() {
    if (!is_finished_) {
      std::vector<std::tuple<int, int, int, int>> triangles_pairs =
          triangles_pairs_;
      for (size_t l = 0; l < level_; l++) {
        std::vector<std::tuple<int, int, int, int>> new_triangles_pairs;
        new_triangles_pairs.reserve(triangles_pairs.size() * 2);
        for (auto& [a_idx, b_idx, c_idx, d_idx] : triangles_pairs) {
          int cd_mid_point_id = get_next_point_id();
          auto cd_mid_point = normalize(points_[c_idx] + points_[d_idx]) * 0.5;
          points_[cd_mid_point_id] = cd_mid_point;

          new_triangles_pairs.push_back(std::tuple<int, int, int, int>{
              a_idx, b_idx, c_idx, cd_mid_point_id});
          new_triangles_pairs.push_back(std::tuple<int, int, int, int>{
              a_idx, b_idx, cd_mid_point_id, d_idx});
        }
        std::swap(new_triangles_pairs, triangles_pairs);
      }
      for (auto& [a_idx, b_idx, c_idx, d_idx] : triangles_pairs) {
        triangles_.push_back(std::tuple<int, int, int>{a_idx, c_idx, d_idx});
        triangles_.push_back(std::tuple<int, int, int>{b_idx, d_idx, c_idx});
      }
      is_finished_ = true;
    }
    Model model;
    model.triangles_ = triangles_;
    model.points_.reserve(points_.size());
    for (int i = 0; i < points_.size(); i++) {
      model.points_.push_back(points_[i]);
    }
    return model;
  }
  bool is_finished_;
  size_t level_;
  int next_point_id_;
  std::vector<std::tuple<int, int, int>> triangles_;
  std::vector<std::tuple<int, int, int, int>> triangles_pairs_;
  std::unordered_map<int, Vec3> points_;
};