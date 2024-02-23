#pragma once
#include <array>
#include <memory>
#include <vector>

#include "mat.hpp"
#include "shader.hpp"
#include "shading.hpp"
#include "vec.hpp"
FragmentStageData from_vertex_stage_to_fragment_stage(
    const VertexStageData &data, int frame_width, int frame_height) {
  Vec2 frame_size{float(frame_width) - 1, float(frame_height) - 1};
  Vec2 ndc_pos = get_vec2_position(data);
  // std::cout << "ndc " << to_string(ndc_pos) << "\n";
  auto frame_pos = 0.5f * (ndc_pos + Vec2{1.0f, 1.0f}) * frame_size;
  frame_pos[1] = frame_size[1] - frame_pos[1];  // flip y
  Vec<int, 2> fragment_position;
  fragment_position[0] = int(frame_pos[0]);
  fragment_position[1] = int(frame_pos[1]);
  return std::make_pair(fragment_position, data);
}

class Renderer {
 public:
  Renderer() {}
  void render(const std::vector<std::vector<float>> &vertices,
              const std::vector<std::tuple<int, int, int>> &triangles) {
    for (auto &[a_idx, b_idx, c_idx] : triangles) {
      auto maybe_new_vertices = geometry_shader_->process(
          vertices[a_idx], vertices[b_idx], vertices[c_idx]);
      for (int i = 2; i < maybe_new_vertices.size(); i += 3) {
        auto a_vertex = vertex_shader_->process(maybe_new_vertices[i - 2]);
        auto b_vertex = vertex_shader_->process(maybe_new_vertices[i - 1]);
        auto c_vertex = vertex_shader_->process(maybe_new_vertices[i]);
        auto frag_a = from_vertex_stage_to_fragment_stage(
            a_vertex, frame_width_, frame_height_);
        auto frag_b = from_vertex_stage_to_fragment_stage(
            b_vertex, frame_width_, frame_height_);
        auto frag_c = from_vertex_stage_to_fragment_stage(
            c_vertex, frame_width_, frame_height_);
        std::vector<std::pair<Vec<int, 2>, std::vector<float>>> new_fragments;
        if (is_draw_framework_) {
          new_fragments =
              Shading::shading_triangle_frame(frag_a, frag_b, frag_c);
        } else {
          new_fragments = Shading::shading_triangle(frag_a, frag_b, frag_c);
        }
        for (auto fragment : new_fragments) {
          auto [pos, color, depth] =
              fragment_shader_->process(fragment);
          set_with_depth_test(pos[0], pos[1], color, depth);
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
  FragmentShader *fragment_shader_{nullptr};
  VertexShader *vertex_shader_{nullptr};
  GeometryShader *geometry_shader_{nullptr};
  Color *color_buffer_{nullptr};
  float *depth_buffer_{nullptr};
  int frame_width_{0};
  int frame_height_{0};
  bool is_draw_framework_{false};
};