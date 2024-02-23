#pragma once
#include "model.hpp"

Model get_cube() {
  Model model;
  model.points_ = std::vector<Vec3>{
      {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f},  {1.0f, 1.0f, 0.0f},
      {-1.0f, 1.0f, 0.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f}};
  model.triangles_ = std::vector<std::tuple<int, int, int>>{
      {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
      {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
  auto point_color = std::vector<Color>{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                        {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
                                        {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                                        {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
  model.attributes_.resize(model.triangles_.size());
  for (int i = 0; i < model.triangles_.size(); i++) {
    auto& [a_idx, b_idx, c_idx] = model.triangles_[i];
    append(model.attributes_[i], point_color[a_idx], point_color[b_idx],
           point_color[c_idx]);
  }
  return model;
}

Model get_uv_cube() {
  Model model;
  model.points_ = std::vector<Vec3>{
      {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f},
      {-1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f}};
  model.triangles_ = std::vector<std::tuple<int, int, int>>{
      {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
      {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
  #define front_norm  0.0f,0.0f,1.0f 
  #define back_norm  0.0f,0.0f,-1.0f 
  #define left_norm  1.0f,0.0f,0.0f 
  #define right_norm  -1.0f,0.0f,0.0f 
  #define top_norm  0.0f,1.0f,0.0f 
  #define down_norm  0.0f,-1.0f,0.0f 
  model.attributes_ = std::vector<TriangleAtrributes>{
      {{0.0f, 1.0f,front_norm}, {1.0f,1.0f,front_norm}, {1.0f,0.0f,front_norm}},{{1.0f,0.0f,front_norm}, {0.0f,0.0f,front_norm}, {0.0f,1.0f,front_norm}},
      {{0.0f, 1.0f,back_norm}, {1.0f,1.0f,back_norm}, {1.0f,0.0f,back_norm}},{{1.0f,0.0f,back_norm}, {0.0f,0.0f,back_norm}, {0.0f,1.0f,back_norm}},
      {{0.0f, 1.0f,left_norm}, {1.0f,1.0f,left_norm}, {1.0f,0.0f,left_norm}},{{1.0f,0.0f,left_norm}, {0.0f,0.0f,left_norm}, {0.0f,1.0f,left_norm}},
      {{0.0f, 1.0f,right_norm}, {1.0f,1.0f,right_norm}, {1.0f,0.0f,right_norm}},{{1.0f,0.0f,right_norm}, {0.0f,0.0f,right_norm}, {0.0f,1.0f,right_norm}},
      {{0.0f, 1.0f,top_norm}, {1.0f,1.0f,top_norm}, {1.0f,0.0f,top_norm}},{{1.0f,0.0f,top_norm}, {0.0f,0.0f,top_norm}, {0.0f,1.0f,top_norm}},
      {{0.0f, 1.0f,down_norm}, {1.0f,1.0f,down_norm}, {1.0f,0.0f,down_norm}},{{1.0f,0.0f,down_norm}, {0.0f,0.0f,down_norm}, {0.0f,1.0f,down_norm}}
  };
  return model;
}
