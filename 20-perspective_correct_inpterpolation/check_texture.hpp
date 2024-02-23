#pragma once
#include <assert.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "check.hpp"
#include "interval.hpp"
#include "vec.hpp"

/*
contract:
  +u = right direction
  +v = down diretion
*/
class CheckerTexture {
 public:
  CheckerTexture() {}
  Color sample(float u, float v) {
    int remainder = (int(u * 10)+ int(v*10))%2;
    return colors_[remainder];
  }
  std::array<Color,2> colors_{Color{0.0f,0.0f,0.0f},Color{1.0f,1.0f,1.0f}};
};