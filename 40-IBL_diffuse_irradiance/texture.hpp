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
  virtual Color sample(float u, float v) { return Color{}; };
  virtual Color sample(float u, float v, float w) { return Color{}; }
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
    //std::cout << x << " " << y <<" "<< width_ << " " << height_ << "\n";
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
    // u = clamp(u,0.0f,1.0f);
    // v = clamp(v,0.0f,1.0f);
    return get(u * (width_ - 1), v * (height_ - 1));
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

class CubeMapTexture : public Texture {
public:
  CubeMapTexture(const std::vector<std::string> &paths) {
    image_textures_.resize(6);
    CHECK(paths.size() == 6)
    for (int i = 0; i < 6; i++) {
      image_textures_[i].reset(new ImageTexture(paths[i]));
    }
  }
  CubeMapTexture(std::vector<std::unique_ptr<ImageTexture>> &image_textures) {
    image_textures_ = std::move(image_textures);
    CHECK(image_textures_.size()==6)
  }
  // face_dir:+x  left:-z right:+z  up:+y down:-y  right
  // face_dir:-x  left:+z right:-z  up:+y down:-y  left
  // face_dir:+z  left:+x right:-x  up:+y down:-y  front
  // face_dir:-z  left:-x right:+x  up:+y down:-y  back
  // face_dir:+y  left:+x right:-x  up:-z down:+z  up
  // face_dir:-y  left:+x right:-x  up:+z down:-z  down
  virtual Color sample(float u, float v, float w) override {
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
        return image_textures_[0]->sample((x + 1.0f) * 0.5f,
                                          1.0 - (y + 1.0f) * 0.5f);
      }
      if (u <= 0.0f) {
        u = -u;
        float x = -w / u;
        float y = v / u;
        return image_textures_[1]->sample((x + 1.0f) * 0.5f,
                                          1.0 - (y + 1.0f) * 0.5f);
      }
    } else if (max_index == 1) {
      // y x z
      // CHECK(std::abs(v)==1.0f)
      if (v > 0.0f) {
        float x = -u / v;
        float y = -w / v;
        return image_textures_[2]->sample((x + 1.0f) * 0.5f,
                                          1.0 - (y + 1.0f) * 0.5f);
      }
      if (v <= -0.0f) {
        v = -v;
        float x = -u / v;
        float y = w / v;
        return image_textures_[3]->sample((x + 1.0f) * 0.5f,
                                          1.0 - (y + 1.0f) * 0.5f);
      }
    } else {
      // z x y
      // CHECK(std::abs(w)==1.0f)
      if (w > 0.0f) {
        float x = -u / w;
        float y = v / w;
        return image_textures_[4]->sample((x + 1.0f) * 0.5f,
                                          1.0f - (y + 1.0f) * 0.5f);
      }
      if (w <= -0.0f) {
        w = -w;
        float x = u / w;
        float y = v / w;
        return image_textures_[5]->sample((x + 1.0f) * 0.5f,
                                          1.0f - (y + 1.0f) * 0.5f);
      }
    }
    CHECK(false); // not reach
    return Color{0.0f, 0.0f, 0.0f};
  }
  std::vector<std::unique_ptr<ImageTexture>> image_textures_;
};

std::shared_ptr<Texture>
make_cube_map_texture(std::vector<std::string> &paths) {
  return std::shared_ptr<Texture>((Texture *)new CubeMapTexture{paths});
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
class ScaleImageTexture : public Texture {
public:
  ScaleImageTexture() {}
  ScaleImageTexture(const std::string &path, float scale) : scale_{scale} {
    load(path);
  }
  void load(const std::string path) {
    int x, y, n;
    unsigned char *data = stbi_load(path.data(), &x, &y, &n, 0);
    CHECK(data != NULL)
    width_ = size_t(x);
    height_ = size_t(y);
    // std::cout << x << " " << y << " " << n << " " << width_ << " " << height_
    //           << "\n";
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
    for (int i = 0; i < width_; i++) {
      for (int j = 0; j < height_; j++) {
        Color color;
        // std::cout << i << " " << j << " " << to_string(color) << "\n";
        for (int c = 0; c < 3;
             c++) { // when using n = 4, color[3] is changed;but
                    // color.size()is 3,so stack is corrupted;
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
    // std::cout << x << " " << y <<" "<< width_ << " " << height_ << "\n";
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
    u = u * scale_;
    u = u - int(u);
    v = v * scale_;
    v = v - int(v);
    return get(u * (width_ - 1), v * (height_ - 1));
  }
  Interval2D get_range() const {
    return Interval2D{0, 0, int(width_), int(height_)};
  }
  size_t width_;
  size_t height_;
  float scale_;
  std::vector<Color> data_;
};

std::shared_ptr<Texture> make_scale_image_texture(const std::string &image_path,
                                                  float scale) {
  return std::shared_ptr<Texture>(
      (Texture *)new ScaleImageTexture{image_path, scale});
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
class EquidistantCylindricalHDRTexture : public Texture {
public:
  EquidistantCylindricalHDRTexture() {}
  EquidistantCylindricalHDRTexture(const std::string &path) { load(path); }
  void load(const std::string path) {
    int x, y, n;
    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf(path.data(), &x, &y, &n, 0);
    CHECK(data != NULL)
    width_ = size_t(x);
    height_ = size_t(y);
    data_.resize(width_ * height_, Color{0.0f, 0.0f, 0.0f});
    for (int i = 0; i < width_; i++) {
      for (int j = 0; j < height_; j++) {
        Color color;
        for (int c = 0; c < 3; c++) {
          color[c] = float(*(data + (j * x + i) * n + c));
        }
        // std::cout << i << " " << j << " " << to_string(color) << "\n";
        set(size_t(i), size_t(j), color);
      }
    }
    stbi_set_flip_vertically_on_load(false);
    stbi_image_free(data);
  }
  void set(size_t x, size_t y, const Color &color) {
    data_[y * width_ + x] = color;
  }
  Color get(size_t x, size_t y) { return data_[y * width_ + x]; }
  virtual Color sample(float u, float v) override {
    // u = clamp(u,0.0f,1.0f);
    // v = clamp(v,0.0f,1.0f);
    return get(u * (width_ - 1), v * (height_ - 1));
  }
  size_t width_;
  size_t height_;
  std::vector<Color> data_;
};

std::shared_ptr<Texture>
make_equidistant_cylindrical_hdr_image_texture(const std::string &image_path) {
  return std::shared_ptr<Texture>(
      (Texture *)new EquidistantCylindricalHDRTexture{image_path});
}