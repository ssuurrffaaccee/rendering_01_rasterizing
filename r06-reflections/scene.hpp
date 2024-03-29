#pragma once
#include <vector>

#include "intersection.hpp"
#include "range.hpp"
#include "sptr.hpp"
class Light;
class Scene : public Intersection {
public:
  Scene() {}
  IntersectionInfo intersection(const Ray &ray,
                                const Range<float> &range) const override {
    IntersectionInfo min_info;
    for (auto &node : nodes_) {
      auto info = node->intersection(ray, range);
      if (info.is_intersected_) {
        min_info.is_intersected_ = true;
        if (min_info.t_ > info.t_) {
          min_info = info;
        }
      }
    }
    return min_info;
  }
  std::vector<SPtr<Light>> lights_;
  std::vector<SPtr<Intersection>> nodes_;
};