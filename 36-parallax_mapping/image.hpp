#pragma once
#include <assert.h>

#include <fstream>
#include <vector>

#include "interval.hpp"
#include "vec.hpp"
/*
contract:
  +x = right direction
  +y = down direction
  width on x axis
  height on y axis
*/
class Image {
 public:
  Image(size_t width, size_t height) : width_{width}, height_{height} {
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
  }
  void set(size_t x, size_t y, const Color &color) {
    data_[y * width_ + x] = color;
  }
  Color get(size_t x, size_t y) { return data_[y * width_ + x]; }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    for (auto &color : data_) {
      auto clamped_color = clamp(color, 0.0f, 1.0f);
      ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
          << ' ' << int(clamped_color[2] * 255) << '\n';
    }
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
};