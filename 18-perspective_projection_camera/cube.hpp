#pragma once
#include "model.hpp"

Model get_cube() {
  Model model;
  model.points_ = std::vector<Vec3>{
      {-0.25f, -0.25f, -0.25f}, {0.25f, -0.25f, -0.25f}, {0.25f, 0.25f, -0.25f},
      {-0.25f, 0.25f, -0.25f},  {-0.25f, -0.25f, 0.25f}, {0.25f, -0.25f, 0.25f},
      {0.25f, 0.25f, 0.25f},    {-0.25f, 0.25f, 0.25f}};
  model.colors_ = std::vector<Color>{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                                     {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
  model.triangles_ = std::vector<std::tuple<int, int, int>>{
      {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
      {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
  return model;
}
Model get_single_color_cube(const Color &color) {
  Model model;
  model.points_ = std::vector<Vec3>{
      {-0.25f, -0.25f, -0.25f}, {0.25f, -0.25f, -0.25f}, {0.25f, 0.25f, -0.25f},
      {-0.25f, 0.25f, -0.25f},  {-0.25f, -0.25f, 0.25f}, {0.25f, -0.25f, 0.25f},
      {0.25f, 0.25f, 0.25f},    {-0.25f, 0.25f, 0.25f}};
  for (auto &_ : model.points_) {
    model.colors_.push_back(color);
  }

  model.triangles_ = std::vector<std::tuple<int, int, int>>{
      {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
      {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
  return model;
}
