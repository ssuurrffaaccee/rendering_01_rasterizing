#pragma once
#include <array>
#include <memory>
#include <vector>

#include "mat.hpp"
#include "vec.hpp"
using VertexStageData = std::vector<float>;
Vec2 get_vec2_position(const VertexStageData &data) {
  return Vec2{data[0], data[1]};
}
Vec3 get_vec3_position(const VertexStageData &data) {
  return Vec3{data[0], data[1], data[2]};
}
Vec4 get_vec4_position(const VertexStageData &data) {
  return Vec4{data[0], data[1], data[2], data[3]};
}

VertexStageData mix(const VertexStageData &data_0,
                    const VertexStageData &data_1, float ratio) {
  size_t size = std::max(data_0.size(), data_1.size());
  VertexStageData out_data;
  out_data.resize(size);
  for (int i = 0; i < size; i++) {
    out_data[i] = ::mix(data_0[i], data_1[i], ratio);
  }
  return out_data;
}

using FragmentStageData = std::pair<Vec<int, 2>, std::vector<float>>;
int get_position_x(const FragmentStageData &data) { return data.first[0]; }
int get_position_y(const FragmentStageData &data) { return data.first[1]; }
static FragmentStageData mix(const FragmentStageData &frag_0,
                             const FragmentStageData &frag_1, float ratio) {
  FragmentStageData out_frag;
  out_frag.first = ::mix(frag_0.first, frag_1.first, ratio);
  out_frag.second = ::mix(frag_0.second, frag_1.second, ratio);
  return out_frag;
}
static FragmentStageData mix_x(const FragmentStageData &frag_0,
                               const FragmentStageData &frag_1, float ratio,
                               int y) {
  FragmentStageData out_frag;
  out_frag.first[0] =
      int(::mix(float(frag_0.first[0]), float(frag_1.first[0]), ratio));
  out_frag.first[1] = y;
  out_frag.second = ::mix(frag_0.second, frag_1.second, ratio);
  return out_frag;
}
static FragmentStageData mix_y(const FragmentStageData &frag_0,
                               const FragmentStageData &frag_1, float ratio,
                               int x) {
  FragmentStageData out_frag;
  out_frag.first[0] = x;
  out_frag.first[1] =
      int(::mix(float(frag_0.first[1]), float(frag_1.first[1]), ratio));
  out_frag.second = ::mix(frag_0.second, frag_1.second, ratio);
  return out_frag;
}
static FragmentStageData mix(const FragmentStageData &frag_0,
                             const FragmentStageData &frag_1, float ratio,
                             int x, int y) {
  FragmentStageData out_frag;
  out_frag.first[0] = x;
  out_frag.first[1] = y;
  out_frag.second = ::mix(frag_0.second, frag_1.second, ratio);
  return out_frag;
}
constexpr size_t GEOMETRY_MAX_GLOGBAL_DATA_NUM = 16;
class GeometryShader {
 public:
  GeometryShader() {}
  virtual ~GeometryShader() {}
  void set_global_data(int index, void *data_ptr) {
    global_datas_[index] = data_ptr;
  }
  virtual std::vector<VertexStageData> process(const VertexStageData &v_0,
                                               const VertexStageData &v_1,
                                               const VertexStageData &v_2) = 0;

 protected:
  void *get_global_data(int index) { return global_datas_[index]; }

 private:
  std::array<void *, GEOMETRY_MAX_GLOGBAL_DATA_NUM> global_datas_;
};
constexpr size_t VERTEX_MAX_GLOGBAL_DATA_NUM = 16;
class VertexShader {
 public:
  VertexShader() {}
  virtual ~VertexShader() {}
  void set_global_data(int index, void *data_ptr) {
    global_datas_[index] = data_ptr;
  }
  virtual VertexStageData process(const VertexStageData &vertex) = 0;

 protected:
  void *get_global_data(int index) { return global_datas_[index]; }

 private:
  std::array<void *, VERTEX_MAX_GLOGBAL_DATA_NUM> global_datas_;
};
constexpr size_t FRAGMENT_MAX_GLOGBAL_DATA_NUM = 16;
class FragmentShader {
 public:
  FragmentShader() {}
  virtual ~FragmentShader() {}
  void set_global_data(int index, void *data_ptr) {
    global_datas_[index] = data_ptr;
  }
  virtual std::tuple<Vec<int, 2>, Color, float> process(
      const FragmentStageData &frag) = 0;

 protected:
  void *get_global_data(int index) { return global_datas_[index]; }

 private:
  std::array<void *, FRAGMENT_MAX_GLOGBAL_DATA_NUM> global_datas_;
};