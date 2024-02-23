#pragma once
#include "check.hpp"
#include "homo.hpp"
#include "mat.hpp"
#include "plane.hpp"
#include "vec.hpp"
struct PerspectiveCameraConfig {
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
class PerspectiveCamera {
public:
  PerspectiveCamera() {}
  void init(const PerspectiveCameraConfig &config) {
    position_ = config.position_;
    coord_base_.bz = normalize(-1.0f * config.forward_);
    coord_base_.bx = normalize(cross(config.up_, coord_base_.bz));
    coord_base_.by = cross(coord_base_.bz, coord_base_.bx);
    world_to_view_transform_ = mmdot<float, 4>(
        to_rotate_transform(transpose<float, 3>(to_matrix(coord_base_))),
        to_position_transform(-1.0f * position_));
    build_perpsective_projection_transform(config);
    build_clipping_planes(config);
  }
  void build_perpsective_projection_transform(
      const PerspectiveCameraConfig &config) {
    CHECK(config.left_ < config.right_)
    CHECK(config.down_ < config.top_)
    CHECK(config.near_ < 0.0f && config.far_ < 0.0f &&
          config.near_ > config.far_)
    // move xy to centor
    Vec3 xy_move = -1.0f * Vec3{0.5f * (config.right_ + config.left_),
                                0.5f * (config.down_ + config.top_), 0.0f};
    // scale xy to [-1.0,1.0]
    Vec3 xy_scale = Vec3{2.0f / (config.right_ - config.left_),
                         2.0f / (config.top_ - config.down_), 1.0f};
    Square4 xy_move_then_scale = mmdot<float, 4>(
        to_scale_transform(xy_scale), to_position_transform(xy_move));
    // projecto xy according z
    Vec3 projection_xy = Vec3{-1.0f * config.near_, -1.0f * config.near_, 1.0f};
    Square4 projection_xy_transofrm = to_scale_transform(projection_xy);
    projection_xy_transofrm[4 * 3 + 3] = 0.0f;
    projection_xy_transofrm[4 * 3 + 2] = -1.0f;

    projection_transform_ =
        mmdot<float, 4>(projection_xy_transofrm, xy_move_then_scale);
    // embedding z_tranform to projection_transform
    // z move to center and scale to [-1.0,1.0]
    Vec3 z_move = -1.0f * Vec3{0.0f, 0.0f, 0.5f * (config.near_ + config.far_)};
    Vec3 z_scale = Vec3{0.0f, 0.0f, 2.0f / (config.near_ - config.far_)};
    auto z_transform = mmdot<float, 4>(to_scale_transform(z_scale),
                                       to_position_transform(z_move));
    // embed z scale
    projection_transform_[4 * 2 + 2] = z_transform[4 * 2 + 2];
    // embed z translate
    projection_transform_[4 * 2 + 3] = z_transform[4 * 2 + 3];
  }
  void build_clipping_planes(const PerspectiveCameraConfig &config) {
    // near
    clipping_planes_.push_back(
        Plane{config.position_ + coord_base_.bz * (config.near_ - 0.001f),
              coord_base_.bz});
    // far
    clipping_planes_.push_back(
        Plane{config.position_ + coord_base_.bz * (config.far_ + 0.001f),
              -1.0f * coord_base_.bz});
    // near plane vector
    Vec3 top_left = coord_base_.bz * config.near_ +
                    coord_base_.bx * config.left_ +
                    coord_base_.by * config.top_;
    Vec3 top_right = coord_base_.bz * config.near_ +
                     coord_base_.bx * config.right_ +
                     coord_base_.by * config.top_;
    Vec3 down_left = coord_base_.bz * config.near_ +
                     coord_base_.bx * config.left_ +
                     coord_base_.by * config.down_;
    Vec3 down_right = coord_base_.bz * config.near_ +
                      coord_base_.bx * config.right_ +
                      coord_base_.by * config.down_;
    Vec3 origin = config.position_; //- 0.001f * coord_base_.bz;
    // top
    clipping_planes_.push_back(
        Plane{origin, normalize(cross(top_right, top_left))});
    // down
    clipping_planes_.push_back(
        Plane{origin, normalize(cross(down_left, down_right))});
    // left
    clipping_planes_.push_back(
        Plane{origin, normalize(cross(top_left, down_left))});
    // right
    clipping_planes_.push_back(
        Plane{origin, normalize(cross(down_right, top_right))});
    // for(auto& plane : clipping_planes_){
    //   std::cout<<to_string(plane.sample_point_)<<to_string(plane.outside_direction_)<<"\n";
    // }
  }
  std::vector<Plane> clipping_planes_;
  SpaceBase3D coord_base_;
  Vec3 position_;
  Square4 world_to_view_transform_;
  Square4 projection_transform_;
};
