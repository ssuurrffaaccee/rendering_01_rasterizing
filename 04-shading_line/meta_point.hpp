#pragma once
#include <array>

#include "vec.hpp"

struct ColorProperty {
  Color color_;
};

ColorProperty mix(const ColorProperty &r, const ColorProperty &l,
                  float &ratio) {
  ColorProperty res;
  res.color_ = ::mix(r.color_, l.color_, ratio);
  return res;
}

template <class Property>
struct MetaPoint {
  std::array<int, 2> pixel_pos;
  Property property;
  static MetaPoint<Property> mix_x(const MetaPoint<Property> &meta_point_l,
                                   const MetaPoint<Property> &meta_point_r,
                                   float ratio, int y) {
    MetaPoint<Property> meta_point;
    meta_point.pixel_pos[0] =
        int(::mix(float(meta_point_l.pixel_pos[0]),
                  float(meta_point_r.pixel_pos[0]), ratio));
    meta_point.pixel_pos[1] = y;
    meta_point.property =
        ::mix(meta_point_l.property, meta_point_r.property, ratio);
    return meta_point;
  }
  static MetaPoint<Property> mix_y(const MetaPoint<Property> &meta_point_l,
                                   const MetaPoint<Property> &meta_point_r,
                                   float ratio, int x) {
    MetaPoint<Property> meta_point;
    meta_point.pixel_pos[0] = x;
    meta_point.pixel_pos[1] =
        int(::mix(float(meta_point_l.pixel_pos[1]),
                  float(meta_point_r.pixel_pos[1]), ratio));
    meta_point.property =
        ::mix(meta_point_l.property, meta_point_r.property, ratio);
    return meta_point;
  }
  static MetaPoint<Property> mix(const MetaPoint<Property> &meta_point_l,
                                 const MetaPoint<Property> &meta_point_r,
                                 float ratio, int x, int y) {
    MetaPoint<Property> meta_point;
    meta_point.pixel_pos[0] = x;
    meta_point.pixel_pos[1] = y;
    meta_point.property =
        ::mix(meta_point_l.property, meta_point_r.property, ratio);
    return meta_point;
  }
  static MetaPoint<Property> mix(const MetaPoint<Property> &meta_point_l,
                                 const MetaPoint<Property> &meta_point_r,
                                 float ratio) {
    MetaPoint<Property> meta_point;
    meta_point.pixel_pos[0] =
        int(::mix(float(meta_point_l.pixel_pos[0]),
                  float(meta_point_r.pixel_pos[0]), ratio));
    meta_point.pixel_pos[1] =
        int(::mix(float(meta_point_l.pixel_pos[1]),
                  float(meta_point_r.pixel_pos[1]), ratio));
    meta_point.property =
        ::mix(meta_point_l.property, meta_point_r.property, ratio);
    return meta_point;
  }
};