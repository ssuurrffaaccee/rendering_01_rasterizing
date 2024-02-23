#pragma once
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "check.hpp"
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
Vec2 get_vec2_position(const VertexStageData &data, int start) {
  return Vec2{data[start], data[start + 1]};
}
Vec3 get_vec3_position(const VertexStageData &data, int start) {
  return Vec3{data[start], data[start + 1], data[start + 2]};
}
Vec4 get_vec4_position(const VertexStageData &data, int start) {
  return Vec4{data[start], data[start + 1], data[start + 2], data[start + 3]};
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

constexpr size_t SHADER_MAX_GLOGBAL_DATA_NUM = 16;
class Shader {
 public:
  Shader() {}
  virtual ~Shader() {}
  void set_global_data(int index, void *data_ptr) {
    global_datas_[index] = data_ptr;
  }
  virtual std::vector<VertexStageData> process_triangle(
      const VertexStageData &v_0, const VertexStageData &v_1,
      const VertexStageData &v_2) = 0;
  virtual VertexStageData process_vertex(const VertexStageData &vertex) = 0;
  virtual std::tuple<Vec<int, 2>, Color, float> process_fragment(
      const FragmentStageData &frag) = 0;
  std::string get_name() { return name_; }
  void set_name(const std::string &name) { name_ = name; }

 protected:
  void *get_global_data(int index) { return global_datas_[index]; }

 private:
  std::array<void *, SHADER_MAX_GLOGBAL_DATA_NUM> global_datas_;
  std::string name_;
};

class ShaderKeeper {
 public:
  static ShaderKeeper &get() {
    static ShaderKeeper keeper;
    return keeper;
  }
  void register_(const std::string &name,
                 const std::function<std::shared_ptr<Shader>()> &func) {
    CHECK(shader_builders_.find(name) == shader_builders_.end())
    shader_builders_[name] = func;
  }
  std::shared_ptr<Shader> build(const std::string &name) {
    auto iter = shader_builders_.find(name);
    CHECK(iter != shader_builders_.end());
    auto shader = iter->second();
    shader->set_name(std::to_string(next_id_));
    next_id_++;
    shader_keeper_[shader->get_name()] = shader;
    return shader;
  }
  void drop(const std::string &name) { shader_keeper_.erase(name); }

 private:
  ShaderKeeper() {}
  std::unordered_map<std::string, std::function<std::shared_ptr<Shader>()>>
      shader_builders_;
  std::unordered_map<std::string, std::shared_ptr<Shader>> shader_keeper_;
  int next_id_{0};
};

#define REGISTER_SHADER(TYPE, NAME, CLASS_NAME)                 \
  class z_##TYPE##_##NAME##_##CLASS_NAME##_Register {           \
   public:                                                      \
    z_##TYPE##_##NAME##_##CLASS_NAME##_Register() {             \
      auto &keeper = ShaderKeeper::get();                       \
      keeper.register_(#NAME, []() {                            \
        return std::shared_ptr<TYPE>((TYPE *)new CLASS_NAME()); \
      });                                                       \
    }                                                           \
  };                                                            \
  static z_##TYPE##_##NAME##_##CLASS_NAME##_Register            \
      z_##TYPE##_##NAME##_##CLASS_NAME##_Register_instance;
