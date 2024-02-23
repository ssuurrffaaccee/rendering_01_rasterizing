#pragma once
#include <assert.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "check.hpp"
#include "interval.hpp"
#include "vec.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

class Texture {
 public:
  Texture() {}
  virtual ~Texture() {}
  virtual Color sample(float u, float v) = 0;
};
/*
contract:
  +x = right direction
  +y = down direction
  +u = right direction
  +v = down diretion
  width on x axis, u axis
  height on y axis, v axis
*/
class BilinearImageTexture : public Texture {
 public:
  BilinearImageTexture() {}
  BilinearImageTexture(const std::string &path) { load(path); }
  void load(const std::string path) {
    int x, y, n;
    unsigned char *data = stbi_load(path.data(), &x, &y, &n, 0);
    CHECK(data != NULL)
    width_ = size_t(x);
    height_ = size_t(y);
    // std::cout << x << " " << y << " " << n << " " << width_ << " " << height_
    //           << "\n";
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});

    for (int i = 0; i < x; i++) {
      for (int j = 0; j < y; j++) {
        Color color;
        for (int c = 0; c < 3; c++) {
          color[c] = float(*(data + (j * x + i) * n + c)) / 255.0f;
        }
        // std::cout << i << " " << j << " " << to_string(color) << "\n";
        set(size_t(i), size_t(j), color);
      }
    }
    stbi_image_free(data);
  }
  void set(size_t x, size_t y, const Color &color) {
    data_[y * width_ + x] = color;
  }
  Color get(size_t x, size_t y) {
    // std::cout << x << " " << y << "\n";
    return data_[y * width_ + x];
  }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    for (auto &color : data_) {
      auto clamped_color = clamp(color, 0.0f, 1.0f);
      ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
          << ' ' << int(clamped_color[2] * 255) << '\n';
    }
  }
  virtual Color sample(float u, float v) override {
    int w = u * (width_ - 1);
    int h = v * (height_ - 1);
    int w_plus_one = clamp(w + 1, 0, width_);
    int h_plus_one = clamp(h + 1, 0, height_);
    Color curent = get(w, h);
    Color right = get(w_plus_one, h);
    Color down = get(w, h_plus_one);
    Color right_down = get(w_plus_one, h_plus_one);
    float frac_w = u * float(width_ - 1) - w;
    float frac_h = v * float(height_ - 1) - h;
    return mix(mix(curent, down, frac_h), mix(right, right_down, frac_h),
               frac_w);
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
};

std::shared_ptr<Texture> make_bilinear_image_texture(
    const std::string &image_path) {
  return std::shared_ptr<Texture>(
      (Texture *)new BilinearImageTexture{image_path});
}
/*
contract:
  +x = right direction
  +y = down direction
  +u = right direction
  +v = down diretion
  width on x axis, u axis
  height on y axis, v axis
*/
class ImageTexture : public Texture {
 public:
  ImageTexture() {}
  ImageTexture(const std::string &path) { load(path); }
  void load(const std::string path) {
    int x, y, n;
    unsigned char *data = stbi_load(path.data(), &x, &y, &n, 0);
    CHECK(data != NULL)
    width_ = size_t(x);
    height_ = size_t(y);
    // std::cout << x << " " << y << " " << n << " " << width_ << " " << height_
    //           << "\n";
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});

    for (int i = 0; i < x; i++) {
      for (int j = 0; j < y; j++) {
        Color color;
        for (int c = 0; c < 3; c++) {
          color[c] = float(*(data + (j * x + i) * n + c)) / 255.0f;
        }
        // std::cout << i << " " << j << " " << to_string(color) << "\n";
        set(size_t(i), size_t(j), color);
      }
    }
    stbi_image_free(data);
  }
  void set(size_t x, size_t y, const Color &color) {
    data_[y * width_ + x] = color;
  }
  Color get(size_t x, size_t y) {
    // std::cout << x << " " << y << "\n";
    return data_[y * width_ + x];
  }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    for (auto &color : data_) {
      auto clamped_color = clamp(color, 0.0f, 1.0f);
      ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
          << ' ' << int(clamped_color[2] * 255) << '\n';
    }
  }
  virtual Color sample(float u, float v) override {
    int w = u * (width_ - 1);
    int h = v * (height_ - 1);
    return get(w, h);
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
};

std::shared_ptr<Texture> make_image_texture(const std::string &image_path) {
  return std::shared_ptr<Texture>((Texture *)new ImageTexture{image_path});
}

/*
contract:
  +u = right direction
  +v = down diretion
*/
class CheckTexture : public Texture {
 public:
  CheckTexture() {}
  virtual Color sample(float u, float v) override {
    int remainder = (int(u * num_) % 2 + int(v * num_) % 2) % 2;
    return colors_[remainder];
  }
  std::array<Color, 2> colors_{Color{0.0f, 0.0f, 0.0f},
                               Color{1.0f, 1.0f, 1.0f}};
  int num_{20};
};

std::shared_ptr<Texture> make_check_texture() {
  return std::shared_ptr<Texture>((Texture *)new CheckTexture{});
}

/*
contract:
  +u = right direction
  +v = down diretion
*/
class ColorTexture : public Texture {
 public:
  ColorTexture(const Color &color) : color_{color} {}
  virtual Color sample(float u, float v) override { return color_; }
  Color color_;
};

std::shared_ptr<Texture> make_color_texture(const Color &color) {
  return std::shared_ptr<Texture>((Texture *)new ColorTexture{color});
}