#pragma once
#include <assert.h>

#include <fstream>
#include <vector>
#include "vec.hpp"
class Image {
 public:
  Image(size_t width, size_t height) : width_{width}, height_{height} {
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
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
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
};