#pragma once
#include "model.hpp"
Model get_base_indicator() {
  Model model;
  model.points_ = std::vector<Vec3>{// x
                                    {0.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    // y
                                    {0.0f, 0.0f, 0.0f},
                                    {0.0f, 1.0f, 0.0f},
                                    // z
                                    {0.0f, 0.0f, 0.0f},
                                    {0.0f, 0.0f, 1.0f}};
  model.colors_ = std::vector<Color>{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
  model.triangles_ =
      std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {2, 2, 3}, {4, 4, 5}};
  return model;
}