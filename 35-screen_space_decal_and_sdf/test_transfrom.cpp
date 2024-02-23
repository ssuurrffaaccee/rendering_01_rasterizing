#include "mat.hpp"
#include <iostream>
int main() {
  Square4 v{0.707107, 0, -0.707107, 0, -0.408248, 0.816497, -0.408248, 0, 0.57735, 0.57735, 0.57735, -17.3205, 0, 0, 0, 1};
  Square4 p{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.020202, 1.0202, 0, 0, -1, 0};
  Square4 vinv = inverse4(v);
  Square4 pinv = inverse4(p);
  Vec4 point{1, 1, 1, 1};
  auto point_in_view = mvdot<float, 4>(v, point);
  auto point_in_projection = mvdot<float, 4>(p, point_in_view);
  auto point_in_view_re = mvdot<float, 4>(pinv, point_in_projection);
  auto pint_re = mvdot<float, 4>(vinv, point_in_view_re);
  std::cout << to_string(point) << "\n";
  std::cout << to_string(point_in_view) << "\n";
  std::cout << to_string(point_in_projection) << "\n";
  std::cout << to_string(point_in_view_re) << "\n";
  std::cout << to_string(pint_re) << "\n";
  float near = -1.0f;
  float far = -100.0f;
  int frame_width = 2000;
  int frame_height = 2000;
  Vec<int, 2> fragment_position;
  // forward
  {
    Vec2 frame_size{float(frame_width) - 1, float(frame_height) - 1};
    Vec2 ndc_pos{point_in_projection[0], point_in_projection[1]};
    auto frame_pos = 0.5f * (ndc_pos + Vec2{1.0f, 1.0f}) * frame_size;
    frame_pos[1] = frame_size[1] - frame_pos[1];
    fragment_position[0] = int(frame_pos[0]);
    fragment_position[1] = int(frame_pos[1]);
    std::cout << to_string(fragment_position) << "\n";
  }
  {
    int x_pos = fragment_position[0];
    int y_pos = fragment_position[1];
    auto width = frame_width - 1;
    auto height = frame_height - 1;
    auto x = (x_pos / float(width)) * 2.0f - 1.0f;
    auto y = ((height - y_pos) / float(height)) * 2.0f - 1.0f;
    auto z = point_in_projection[2];
    auto w = (z / (2.0f / (near - far))) +
             0.5f * (near + far);
    auto point_in_projection = Vec4{x, y, z, -w};
    std::cout<<"\n";
    std::cout << to_string(point_in_projection) << "\n";
    auto view_space_point =
        mvdot<float, 4>(pinv, point_in_projection);
    std::cout << to_string(view_space_point) << "\n";
    auto world_space_point =
        mvdot<float, 4>(vinv, view_space_point);
    std::cout << to_string(world_space_point) << "\n";
  }
  return 0;
}