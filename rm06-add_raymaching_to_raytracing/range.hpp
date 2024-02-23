#pragma once
template <typename T> struct Range {
  Range(T left, T right) : left_{left}, right_{right} {}
  bool is_in(T v) const { return v >= left_ && v < right_; }
  bool is_out(T v) const { return !is_in(v); }
  T left_;
  T right_;
};