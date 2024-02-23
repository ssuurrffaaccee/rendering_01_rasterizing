#pragma once

#include "ray.hpp"

struct CameraConfig {
  Vec3 origin_;
  Vec3 forward_;
  Vec3 up_;
  float canvas_width_;
  float canvas_height_;
  float canvas_forward_distane_;
  int image_width_;
  int image_height_;
};
struct RayGenerator {
  bool empty() {
    if (cur_y_ < image_height_ && cur_x_ < image_width_) {
      return false;
    }
    return true;
  }
  std::pair<Vec<int, 2>, Ray> next() {
    auto ray_direction = canvas_origin_on_view_base +
                         (cur_x_ / float(image_width_)) * width_vec_ +
                         (cur_y_ / float(image_height_)) * height_vec_;
    auto ray = Ray{origin_, ray_direction};
    auto res = std::make_pair(Vec<int, 2>{cur_x_, cur_y_}, ray);
    // update;
    cur_x_++;
    cur_y_ += (cur_x_ / image_width_);
    cur_x_ = cur_x_ % image_width_;
    return res;
  }
  int cur_x_;
  int cur_y_;
  Vec3 origin_;
  Vec3 canvas_origin_on_view_base;
  int image_width_;
  int image_height_;
  Vec3 width_vec_;
  Vec3 height_vec_;
};

class Camera {
 public:
  Camera() {}
  void init(const CameraConfig &config) {
    origin_ = config.origin_;
    coord_base_.bz = normalize(-1.0f * config.forward_);
    coord_base_.bx = normalize(cross(config.up_, coord_base_.bz));
    coord_base_.by = cross(coord_base_.bz, coord_base_.bx);
    width_vec_ = coord_base_.bx * config.canvas_width_;
    height_vec_ = -1.0f * coord_base_.by * config.canvas_height_;
    canvas_forward_distance_ = config.canvas_forward_distane_;
    image_width_ = config.image_width_;
    image_height_ = config.image_height_;
  }
  RayGenerator get_ray_generator() {
    RayGenerator ray_generator;
    ray_generator.origin_ = origin_;
    ray_generator.canvas_origin_on_view_base =
        (-1.0f) * coord_base_.bz * canvas_forward_distance_ -
        0.5 * (width_vec_ + height_vec_);
    ray_generator.image_height_ = image_height_;
    ray_generator.image_width_ = image_width_;
    ray_generator.cur_x_ = 0;
    ray_generator.cur_y_ = 0;
    ray_generator.width_vec_ = width_vec_;
    ray_generator.height_vec_ = height_vec_;
    return ray_generator;
  }
  SpaceBase3D coord_base_;
  Vec3 origin_;
  Vec3 width_vec_;
  Vec3 height_vec_;
  float canvas_forward_distance_;
  int image_width_;
  int image_height_;
};
