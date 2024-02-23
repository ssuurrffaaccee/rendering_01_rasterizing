#pragma once
#include <vector>

#include "intersection.hpp"
#include "sptr.hpp"
class Scene : public Intersection {
 public:
  Scene() {}
  IntersectionInfo intersection(const Ray& ray) override {
    IntersectionInfo min_info;
    for (auto& node : nodes_) {
      auto info = node->intersection(ray);
      if (info.is_intersected_) {
        min_info.is_intersected_ = true;
        min_info.t_ = min_info.t_ < info.t_ ? min_info.t_ : info.t_;
        min_info.color_ = info.color_;
      }
    }
    return min_info;
  }
  std::vector<SPtr<Intersection>> nodes_;
};