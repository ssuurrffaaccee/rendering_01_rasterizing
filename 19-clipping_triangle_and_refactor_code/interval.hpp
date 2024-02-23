#pragma once
class Interval2D {
 public:
  explicit Interval2D(int min_x, int min_y, int max_x, int max_y)
      : min_x_{min_x}, min_y_{min_y}, max_y_{max_y}, max_x_{max_x} {}
  bool is_in(int x, int y) const {
    if (x >= min_x_ && x < max_x_ && y >= min_y_ && y < max_y_) {
      return true;
    }
    return false;
  }
  bool is_out(int x, int y) const { return !is_in(x, y); }
  bool is_in_x(int x) const {
    if (x >= min_x_ && x < max_x_) {
      return true;
    }
    return false;
  }
  bool is_out_x(int x) const { return !is_in_x(x); }
  bool is_out_y(int y) const { return !is_in_y(y); }
  bool is_in_y(int y) const {
    if (y >= min_y_ && y < max_y_) {
      return true;
    }
    return false;
  }
  int get_min_x() const { return min_x_; }
  int get_max_x() const { return max_x_; }
  int get_min_y() const { return min_y_; }
  int get_max_y() const { return max_y_; }

 private:
  int min_x_;
  int min_y_;
  int max_x_;
  int max_y_;
};