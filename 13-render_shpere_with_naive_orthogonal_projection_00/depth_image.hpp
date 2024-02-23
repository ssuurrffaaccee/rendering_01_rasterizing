#pragma once
#include <assert.h>

#include <fstream>
#include <vector>

#include "interval.hpp"
#include "vec.hpp"
#include <iostream>
class DepthImage {
 public:
  DepthImage(size_t width, size_t height) : width_{width}, height_{height} {
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
    depth_buffer_.resize(width_ * height_, std::numeric_limits<float>::max());
  }
  void set(size_t x, size_t y, const Color &color) {
    data_[x * width_ + y] = color;
  }
  Color get(size_t x, size_t y) { return data_[x * width_ + y]; }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    for (auto &color : data_) {
      auto clamped_color = clamp(color, 0.0f, 1.0f);
      ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
          << ' ' << int(clamped_color[2] * 255) << '\n';
    }
  }
  void set_with_depth_test(size_t x, size_t y, const Color &color,
                           float depth) {
    if (depth <= depth_buffer_[x * width_ + y]) {
      depth_buffer_[x * width_ + y] = depth;
      data_[x * width_ + y] = color;
    }
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
  std::vector<float> depth_buffer_;
};