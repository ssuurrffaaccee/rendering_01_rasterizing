#pragma once
#include <assert.h>

#include <fstream>
#include <iostream>
#include <memory>
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
class DepthImage {
 public:
  DepthImage(size_t width, size_t height) : width_{width}, height_{height} {
    data_ = std::make_shared<std::vector<Color>>();
    depth_buffer_ = std::make_shared<std::vector<float>>();
    data_->resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
    depth_buffer_->resize(width_ * height_,
                          std::numeric_limits<float>::lowest());
  }
  void set(size_t x, size_t y, const Color &color) {
    data_->operator[](y * width_ + x) = color;
  }
  Color get(size_t x, size_t y) { return data_->operator[](y * width_ + x); }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    for (auto &color : *data_) {
      auto clamped_color = clamp(color, 0.0f, 1.0f);
      ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
          << ' ' << int(clamped_color[2] * 255) << '\n';
    }
  }
  void set_with_depth_test(size_t x, size_t y, const Color &color,
                           float depth) {
    if (depth > depth_buffer_->operator[](y * width_ + x)) {
      depth_buffer_->operator[](y * width_ + x) = depth;
      data_->operator[](y * width_ + x) = color;
    }
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  std::shared_ptr<std::vector<Color>> data_;
  std::shared_ptr<std::vector<float>> depth_buffer_;
};
/*
contract:
  +x = right direction
  +y = down direction
  width on x axis
  height on y axis
*/
class SSAADepthImage {
 public:
  SSAADepthImage(size_t width, size_t height, size_t SSAA)
      : SSAA_width_{width * SSAA}, SSAA_height_{height * SSAA}, SSAA_{SSAA} {
    CHECK(SSAA >= 1)
    data_ = std::make_shared<std::vector<Color>>();
    depth_buffer_ = std::make_shared<std::vector<float>>();
    data_->resize(SSAA_width_ * SSAA_height_, Color{0.0f, 0.0f, 0.0f});
    depth_buffer_->resize(SSAA_width_ * SSAA_height_,
                          std::numeric_limits<float>::lowest());
  }
  void set(size_t x, size_t y, const Color &color) {
    data_->operator[](y * SSAA_width_ + x) = color;
  }
  Color get(size_t x, size_t y) {
    // std::cout<<"get "<<x<<" "<<y<<"\n";
    return data_->operator[](y * SSAA_width_ + x);
  }
  void set_with_depth_test(size_t x, size_t y, const Color &color,
                           float depth) {
    CHECK_WITH_INFO(x >= 0 && x < SSAA_width_, std::to_string(x))
    CHECK_WITH_INFO(y >= 0 && y < SSAA_height_, std::to_string(y))
    auto depth_buffer_ptr = depth_buffer_.get()->data();
    auto color_buffer_ptr = data_.get()->data();
    if (depth > *(depth_buffer_ptr + y * SSAA_width_ + x)) {
      *(depth_buffer_ptr + y * SSAA_width_ + x) = depth;
      *(color_buffer_ptr + y * SSAA_width_ + x) = color;
    }
  }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    size_t width_ = SSAA_width_ / SSAA_;
    size_t height_ = SSAA_height_ / SSAA_;
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    size_t SSAA_sample_num = SSAA_ * SSAA_;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        Color color = Color{0.0f, 0.0f, 0.0f};
        for (size_t i = 0; i < SSAA_; i++) {
          for (size_t j = 0; j < SSAA_; j++) {
            color += get(x * SSAA_ + i, y * SSAA_ + j) / SSAA_sample_num;
          }
        }
        auto clamped_color = clamp(color, 0.0f, 1.0f);
        ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
            << ' ' << int(clamped_color[2] * 255) << '\n';
      }
    }
  }
  void hdr_dump(const std::string name, float exposure) {
    std::ofstream ofs(name + ".ppm");
    size_t width_ = SSAA_width_ / SSAA_;
    size_t height_ = SSAA_height_ / SSAA_;
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    size_t SSAA_sample_num = SSAA_ * SSAA_;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        Color hdr_color = Color{0.0f, 0.0f, 0.0f};
        for (size_t i = 0; i < SSAA_; i++) {
          for (size_t j = 0; j < SSAA_; j++) {
            hdr_color += get(x * SSAA_ + i, y * SSAA_ + j) / SSAA_sample_num;
          }
        }
        auto color = Vec3{1.0f, 1.0f, 1.0f} - exp(-1.0f * hdr_color * exposure);
        auto clamped_color = clamp(color, 0.0f, 1.0f);
        ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
            << ' ' << int(clamped_color[2] * 255) << '\n';
      }
    }
  }
  size_t SSAA_width_;
  size_t SSAA_height_;
  size_t SSAA_;
  std::shared_ptr<std::vector<Color>> data_;
  std::shared_ptr<std::vector<float>> depth_buffer_;
};

/*
contract:
  +x = right direction
  +y = down direction
  width on x axis
  height on y axis
*/
class SSAAOnlyDepthImage {
 public:
  SSAAOnlyDepthImage(size_t width, size_t height, size_t SSAA)
      : SSAA_width_{width * SSAA}, SSAA_height_{height * SSAA}, SSAA_{SSAA} {
    CHECK(SSAA >= 1)
    depth_buffer_ = std::make_shared<std::vector<float>>();
    depth_buffer_->resize(SSAA_width_ * SSAA_height_,
                          std::numeric_limits<float>::lowest());
  }
  float get(size_t x, size_t y) const {
    return depth_buffer_->operator[](y * SSAA_width_ + x);
  }
  float get_by_uv(float u, float v) const {
    size_t x = (SSAA_width_ - 1) * u;
    size_t y = (SSAA_height_ - 1) * v;
    return depth_buffer_->operator[](y * SSAA_width_ + x);
  }
  void dump(const std::string name) {
    std::ofstream ofs(name + ".ppm");
    size_t width_ = SSAA_width_ / SSAA_;
    size_t height_ = SSAA_height_ / SSAA_;
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    size_t SSAA_sample_num = SSAA_ * SSAA_;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        float depth = 0.0f;
        for (size_t i = 0; i < SSAA_; i++) {
          for (size_t j = 0; j < SSAA_; j++) {
            depth += get(x * SSAA_ + i, y * SSAA_ + j) / SSAA_sample_num;
          }
        }
        auto clamped_depth = clamp(-0.5f * (depth - 1.0f), 0.0f, 1.0f);
        auto clamped_color = gray(clamped_depth);
        ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
            << ' ' << int(clamped_color[2] * 255) << '\n';
      }
    }
  }
  void set_with_depth_test(size_t x, size_t y, float depth) {
    if (depth > depth_buffer_->operator[](y * SSAA_width_ + x)) {
      depth_buffer_->operator[](y * SSAA_width_ + x) = depth;
    }
  }
  size_t SSAA_width_;
  size_t SSAA_height_;
  size_t SSAA_;
  std::shared_ptr<std::vector<float>> depth_buffer_;
};

/*
contract:
  +x = right direction
  +y = down direction
  width on x axis
  height on y axis
*/
class SSAAGBufferDepthImage {
 public:
  using GBuffer = std::vector<float>;
  SSAAGBufferDepthImage(size_t width, size_t height, size_t SSAA)
      : SSAA_width_{width * SSAA}, SSAA_height_{height * SSAA}, SSAA_{SSAA} {
    CHECK(SSAA >= 2)
    data_ = std::make_shared<std::vector<std::vector<float>>>();
    data_->resize(SSAA_width_ * SSAA_height_, std::vector<float>{});
    depth_buffer_ = std::make_shared<std::vector<float>>();
    depth_buffer_->resize(SSAA_width_ * SSAA_height_,
                          std::numeric_limits<float>::lowest());
  }
  void set(size_t x, size_t y, const GBuffer &g_buffer) {
    data_->operator[](y * SSAA_width_ + x) = g_buffer;
  }
  GBuffer get(size_t x, size_t y) {
    return data_->operator[](y * SSAA_width_ + x);
  }
  Vec3 get(size_t x, size_t y, int start) {
    auto &g_buffer = data_->operator[](y * SSAA_width_ + x);
    if (g_buffer.size() < start + 3) {
      g_buffer.resize(start + 3);
    }
    return get_vec3_position(g_buffer, start);
  }
  void set_with_depth_test(size_t x, size_t y, const GBuffer &g_buffer,
                           float depth) {
    if (depth > depth_buffer_->operator[](y * SSAA_width_ + x)) {
      depth_buffer_->operator[](y * SSAA_width_ + x) = depth;
      data_->operator[](y * SSAA_width_ + x) = g_buffer;
    }
  }
  void dump(const std::string name, int start) {
    std::ofstream ofs(name + ".ppm");
    size_t width_ = SSAA_width_ / SSAA_;
    size_t height_ = SSAA_height_ / SSAA_;
    ofs << "P3\n" << width_ << ' ' << height_ << "\n255\n";
    size_t SSAA_sample_num = SSAA_ * SSAA_;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        Color color = Color{0.0f, 0.0f, 0.0f};
        for (size_t i = 0; i < SSAA_; i++) {
          for (size_t j = 0; j < SSAA_; j++) {
            color += get(x * SSAA_ + i, y * SSAA_ + j, start) / SSAA_sample_num;
          }
        }
        auto clamped_color = clamp(color, 0.0f, 1.0f);
        ofs << int(clamped_color[0] * 255) << ' ' << int(clamped_color[1] * 255)
            << ' ' << int(clamped_color[2] * 255) << '\n';
      }
    }
  }
  size_t SSAA_width_;
  size_t SSAA_height_;
  size_t SSAA_;
  std::shared_ptr<std::vector<std::vector<float>>> data_;
  std::shared_ptr<std::vector<float>> depth_buffer_;
};

class OnlyDepthCubaImage {
 public:
  OnlyDepthCubaImage(size_t width, size_t height,
                     std::vector<std::unique_ptr<SSAAOnlyDepthImage>> &images)
      : width_{width}, height_{height}, depth_images_{std::move(images)} {
    CHECK(depth_images_.size() == 6)
    for (auto &image : depth_images_) {
      CHECK(image != nullptr)
      CHECK(image->SSAA_ == 1)
      CHECK(image->SSAA_height_ == height_)
      CHECK(image->SSAA_width_ == width_)
    }
  }
  // face_dir:+x  left:-z right:+z  up:+y down:-y  right
  // face_dir:-x  left:+z right:-z  up:+y down:-y  left
  // face_dir:+z  left:+x right:-x  up:+y down:-y  front
  // face_dir:-z  left:-x right:+x  up:+y down:-y  back
  // face_dir:+y  left:+x right:-x  up:-z down:+z  up
  // face_dir:-y  left:+x right:-x  up:+z down:-z  down
  float get(float u, float v, float w) {
    auto uvw = normalize(Vec3{u, v, w});
    u = uvw[0];
    v = uvw[1];
    w = uvw[2];
    int max_index = arg_abs_max(u, v, w);
    if (max_index == 0) {
      // CHECK(std::abs(u)==1.0f)
      if (u > 0.0f) {
        float x = w / u;
        float y = v / u;
        return depth_images_[0]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0 - (y + 1.0f) * 0.5f);
      }
      if (u <= 0.0f) {
        u = -u;
        float x = -w / u;
        float y = v / u;
        return depth_images_[1]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0 - (y + 1.0f) * 0.5f);
      }
    } else if (max_index == 1) {
      // y x z
      // CHECK(std::abs(v)==1.0f)
      if (v > 0.0f) {
        float x = -u / v;
        float y = -w / v;
        return depth_images_[2]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0 - (y + 1.0f) * 0.5f);
      }
      if (v <= -0.0f) {
        v = -v;
        float x = -u / v;
        float y = w / v;
        return depth_images_[3]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0 - (y + 1.0f) * 0.5f);
      }
    } else {
      // z x y
      // CHECK(std::abs(w)==1.0f)
      if (w > 0.0f) {
        float x = -u / w;
        float y = v / w;
        return depth_images_[4]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0f - (y + 1.0f) * 0.5f);
      }
      if (w <= -0.0f) {
        w = -w;
        float x = u / w;
        float y = v / w;
        return depth_images_[5]->get_by_uv((x + 1.0f) * 0.5f,
                                           1.0f - (y + 1.0f) * 0.5f);
      }
    }
    CHECK(false);  // not reach
    return 0.0f;
  }
  std::vector<std::unique_ptr<SSAAOnlyDepthImage>> depth_images_;

  size_t width_;
  size_t height_;
};
