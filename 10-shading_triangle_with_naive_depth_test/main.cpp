#include <iostream>

#include "depth_image.hpp"
#include "ndc.hpp"
#include "shading.hpp"
int main() {
  try {
    size_t height = 1000;
    size_t width = 1000;
    DepthImage image{width, height};
    Interval2D image_range = image.get_range();
    Shading shading{image_range};

    // near triangle
    {
      float depth = 0.1f;
      Vec2 pos_a{0.25f, 0.25f};
      Color color_a{1.0f, 0.0f, 0.0f};
      Vec2 pos_b{-0.25f, 0.0f};
      Color color_b{1.0f, 0.0f, 0.0f};
      Vec2 pos_c{-0.25f, -0.25f};
      Color color_c{1.0f, 0.0f, 0.0f};
      MetaPoint<ColorDepthProperty> a{NDC_to_Image(pos_a, image_range),
                                      ColorDepthProperty{depth, color_a}};
      MetaPoint<ColorDepthProperty> b{NDC_to_Image(pos_b, image_range),
                                      ColorDepthProperty{depth, color_b}};
      MetaPoint<ColorDepthProperty> c{NDC_to_Image(pos_c, image_range),
                                      ColorDepthProperty{depth, color_c}};
      {
        auto meta_points = shading.shading_triangle(a, b, c);
        for (auto &p : meta_points) {
          image.set_with_depth_test(p.pixel_pos[0], p.pixel_pos[1],
                                    p.property.color_, p.property.depth_);
        }
      }
    }
    // far triangle
    {
      float depth = 0.2f;
      Vec2 pos_a{-0.25f, 0.25f};
      Color color_a{0.0f, 1.0f, 0.0f};
      Vec2 pos_b{0.25f, 0.0f};
      Color color_b{0.0f, 1.0f, 0.0f};
      Vec2 pos_c{0.25f, -0.25f};
      Color color_c{0.0f, 1.0f, 0.0f};
      MetaPoint<ColorDepthProperty> a{NDC_to_Image(pos_a, image_range),
                                      ColorDepthProperty{depth, color_a}};
      MetaPoint<ColorDepthProperty> b{NDC_to_Image(pos_b, image_range),
                                      ColorDepthProperty{depth, color_b}};
      MetaPoint<ColorDepthProperty> c{NDC_to_Image(pos_c, image_range),
                                      ColorDepthProperty{depth, color_c}};
      {
        auto meta_points = shading.shading_triangle(a, b, c);
        for (auto &p : meta_points) {
          image.set_with_depth_test(p.pixel_pos[0], p.pixel_pos[1],
                                    p.property.color_, p.property.depth_);
        }
      }
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}