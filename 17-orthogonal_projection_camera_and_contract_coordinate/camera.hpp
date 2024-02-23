#pragma once
#include "check.hpp"
#include "homo.hpp"
#include "mat.hpp"
#include "vec.hpp"
struct OrthogonalCameraConfig {
  Vec3 position_;
  Vec3 forward_;
  Vec3 up_;
  float far_;
  float near_;
  float left_;
  float right_;
  float top_;
  float down_;
};

Square3 to_matrix(const SpaceBase3D &base) {
  Square3 res;
  res.fill(0.0f);
  int offset = 0;
  for (int i = 0; i < 3; i++) {
    res[3 * i + offset] = base.bx[i];
  }
  offset = 1;
  for (int i = 0; i < 3; i++) {
    res[3 * i + offset] = base.by[i];
  }
  offset = 2;
  for (int i = 0; i < 3; i++) {
    res[3 * i + offset] = base.bz[i];
  }
  return res;
}
/*
contract:
  -z = forwrade direction
  +y = up direction
  +x = cross(+y,+z)
*/
class OrthogonalCamera {
public:
  OrthogonalCamera() {}
  void init(const OrthogonalCameraConfig &config) {
    position_ = config.position_;
    coord_base_.bz = normalize(-1.0f * config.forward_);
    coord_base_.bx = normalize(cross(config.up_, coord_base_.bz));
    coord_base_.by = cross(coord_base_.bz, coord_base_.bx);
    world_to_view_transform_ = mmdot<float, 4>(
        to_rotate_transform(transpose<float, 3>(to_matrix(coord_base_))),
        to_position_transform(-1.0f * position_));
    build_orthogonal_projection_transform(config);
  }
  void
  build_orthogonal_projection_transform(const OrthogonalCameraConfig &config) {
    CHECK(config.left_ < config.right_)
    CHECK(config.down_ < config.top_)
    CHECK(config.near_ > config.far_)
    Vec3 centor = Vec3{0.5f * (config.right_ + config.left_),
                       0.5f * (config.down_ + config.top_), config.near_};
    Vec3 scale = Vec3{1.0f / (config.right_ - config.left_),
                      1.0f / (config.top_ - config.down_),
                      1.0f / (config.near_ - config.far_)};
    projection_transform_ = mmdot<float, 4>(
        to_scale_transform(scale), to_position_transform(-1.0f * centor));
  }
  SpaceBase3D coord_base_;
  Vec3 position_;
  Square4 world_to_view_transform_;
  Square4 projection_transform_;
};
