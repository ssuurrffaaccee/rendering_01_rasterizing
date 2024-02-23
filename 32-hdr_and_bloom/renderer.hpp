#pragma once
#include <array>
#include <memory>
#include <vector>

#include "back_face_culling.hpp"
#include "mat.hpp"
#include "shader.hpp"
#include "shading.hpp"
#include "vec.hpp"
#include "model.hpp"

FragmentStageData from_vertex_stage_to_fragment_stage(
    const VertexStageData &data, int frame_width, int frame_height) {
  Vec2 frame_size{float(frame_width) - 1, float(frame_height) - 1};
  Vec2 ndc_pos = clamp(get_vec2_position(data), -1.0f, 1.0f);
  // Vec2 ndc_pos = get_vec2_position(data);
  auto frame_pos = 0.5f * (ndc_pos + Vec2{1.0f, 1.0f}) * frame_size;
  frame_pos[1] = frame_size[1] - frame_pos[1];  // flip y
  Vec<int, 2> fragment_position;
  fragment_position[0] = int(frame_pos[0]);
  fragment_position[1] = int(frame_pos[1]);
  return std::make_pair(fragment_position, data);
}
void inverse_perspective_correct(FragmentStageData &data) {
  float one_over_w = data.second[3];
  for (int i = 0; i < data.second.size(); i++) {
    data.second[i] /= one_over_w;
  }
}
VertexStageData append(const VertexStageData &data,
                       const std::vector<float> &tail) {
  auto new_data = data;
  new_data.insert(new_data.end(), tail.begin(), tail.end());
  return new_data;
}
class Renderer {
 public:
  Renderer() {}
  // void render(const std::vector<std::vector<float>> &vertices,
  //             const std::vector<std::tuple<int, int, int>> &triangles) {
  //   for (auto &[a_idx, b_idx, c_idx] : triangles) {
  //     auto maybe_new_vertices = geometry_shader_->process(
  //         vertices[a_idx], vertices[b_idx], vertices[c_idx]);
  //     draw_triangle(maybe_new_vertices);
  //   }
  // }
  void render(const std::vector<std::vector<float>> &vertices,
              const std::vector<std::tuple<int, int, int>> &triangles,
              const std::vector<TriangleAtrributes> &attrs) {
    auto triangle_size = std::max(triangles.size(), attrs.size());
    for (int i = 0; i < triangle_size; i++) {
      auto &[a_idx, b_idx, c_idx] = triangles[i];
      auto &[a_attr, b_attr, c_attr] = attrs[i];
      auto a_vert = append(vertices[a_idx], a_attr);
      auto b_vert = append(vertices[b_idx], b_attr);
      auto c_vert = append(vertices[c_idx], c_attr);
      //std::cout<<to_string(a_vert)<<to_string(b_vert)<<to_string(c_vert)<<"\n";
      auto maybe_new_vertices =
          shader_->process_triangle(a_vert, b_vert, c_vert);
      //std::cout<<maybe_new_vertices.size()<<"\n";
      draw_triangle(maybe_new_vertices);
    }
  }
  void draw_triangle(std::vector<VertexStageData> &vertices) {
    for (int i = 2; i < vertices.size(); i += 3) {
      // std::cout<<"cli "<<to_string(vertices[i - 2])<<to_string(vertices[i -
      // 1])<<to_string(vertices[i - 0])<<"\n";
      auto a_vertex = shader_->process_vertex(vertices[i - 2]);
      auto b_vertex = shader_->process_vertex(vertices[i - 1]);
      auto c_vertex = shader_->process_vertex(vertices[i]);
      //std::cout<<to_string(a_vertex)<<to_string(b_vertex)<<to_string(c_vertex)<<"\n";
      if (enable_back_face_culling_ &&
          is_back_face(a_vertex, b_vertex, c_vertex)) {
        continue;
      }
      auto frag_a = from_vertex_stage_to_fragment_stage(a_vertex, frame_width_,
                                                        frame_height_);
      auto frag_b = from_vertex_stage_to_fragment_stage(b_vertex, frame_width_,
                                                        frame_height_);
      auto frag_c = from_vertex_stage_to_fragment_stage(c_vertex, frame_width_,
                                                        frame_height_);
      std::vector<std::pair<Vec<int, 2>, std::vector<float>>> new_fragments;
      if (is_draw_framework_) {
        new_fragments = Shading::shading_triangle_frame(frag_a, frag_b, frag_c);
      } else {
        new_fragments = Shading::shading_triangle(frag_a, frag_b, frag_c);
      }
      for (auto fragment : new_fragments) {
          inverse_perspective_correct(fragment);
        auto [pos, color, depth] = shader_->process_fragment(fragment);
        if (!result_bypass_) {
          if (enable_depth_test_) {
            if (!enable_only_depth_test_) {
              set_with_depth_test(pos[0], pos[1], color, depth);
            } else {
              set_only_with_depth_test(pos[0], pos[1], depth);
            }
          } else {
            set(pos[0], pos[1], color);
          }
        }
      }
    }
  }
  void set_with_depth_test(size_t x, size_t y, const Color &color,
                           float depth) {
    CHECK_WITH_INFO(x >= 0 && x < frame_width_, std::to_string(x))
    CHECK_WITH_INFO(y >= 0 && y < frame_height_, std::to_string(y))
    if (depth > *(depth_buffer_ + y * frame_width_ + x)) {
      *(depth_buffer_ + y * frame_width_ + x) = depth;
      *(color_buffer_ + y * frame_width_ + x) = color;
    }
  }
  void set_only_with_depth_test(size_t x, size_t y, float depth) {
    CHECK_WITH_INFO(x >= 0 && x < frame_width_, std::to_string(x))
    CHECK_WITH_INFO(y >= 0 && y < frame_height_, std::to_string(y))
    if (depth > *(depth_buffer_ + y * frame_width_ + x)) {
      *(depth_buffer_ + y * frame_width_ + x) = depth;
    }
  }
  void set(size_t x, size_t y, const Color &color) {
    CHECK_WITH_INFO(x >= 0 && x < frame_width_, std::to_string(x))
    CHECK_WITH_INFO(y >= 0 && y < frame_height_, std::to_string(y))
    *(color_buffer_ + y * frame_width_ + x) = color;
  }
  Shader *shader_{nullptr};
  // VertexShader *vertex_shader_{nullptr};
  // GeometryShader *geometry_shader_{nullptr};
  Color *color_buffer_{nullptr};
  float *depth_buffer_{nullptr};
  int frame_width_{0};
  int frame_height_{0};
  int SSAA{2};
  bool is_draw_framework_{false};
  bool enable_back_face_culling_{true};
  bool enable_depth_test_{true};
  bool enable_only_depth_test_{false};
  bool result_bypass_{false};
};