#pragma once
#include "clipping.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"
struct DecalCanvasCoordination {
  DecalCanvasCoordination(const Vec3 &origin_point, const Vec3 &x_axis,
                          const Vec3 &y_axis, float width, float height)
      : origin_point_{origin_point}, x_axis_{normalize(x_axis)},
        y_axis_{normalize(y_axis)}, width_{width}, height_{height} {
    normal_ = cross(x_axis_, y_axis_);
  }
  Vec3 origin_point_;
  Vec3 x_axis_;
  Vec3 y_axis_;
  Vec3 normal_;
  float width_;
  float height_;
  bool projection(const Vec3 &point, const Vec3 &point_norm,
                  Vec2 &canvss_coard) {
    if (abs(dot(point_norm, normal_)) <= 0.01) {
      return false;
    }
    auto based_vector = point - origin_point_;
    auto inner_vector = based_vector - dot(normal_, based_vector) * normal_;
    auto x = dot(inner_vector, x_axis_);
    if (x < 0.0f || x >= width_) {
      return false;
    }
    canvss_coard[0] = x / width_;
    auto y = dot(inner_vector, y_axis_);
    if (y < 0.0f || y >= height_) {
      return false;
    }
    canvss_coard[1] = y / height_;
    return true;
  }
  bool projection(const Vec3 &point, Vec2 &canvss_coard) {
    auto based_vector = point - origin_point_;
    auto inner_vector = based_vector - dot(normal_, based_vector) * normal_;
    auto x = dot(inner_vector, x_axis_);
    if (x < 0.0f || x >= width_) {
      return false;
    }
    canvss_coard[0] = x / width_;
    auto y = dot(inner_vector, y_axis_);
    if (y < 0.0f || y >= height_) {
      return false;
    }
    canvss_coard[1] = y / height_;
    return true;
  }
};
class DecalTextureBlinnPhongShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    Square4 &model_transform = *(Square4 *)get_global_data(0);
    std::vector<Plane> &clipping_planes =
        *(std::vector<Plane> *)get_global_data(1);
    auto world_v_0 = transform_to_world(model_transform, v_0);
    auto world_v_1 = transform_to_world(model_transform, v_1);
    auto world_v_2 = transform_to_world(model_transform, v_2);
    Square3 normal_matrix =
        transpose<float, 3>(inverse3(shrink(model_transform)));
    transform_norm_to_world_inplace(normal_matrix, world_v_0, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_1, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_2, 6);
    // std::cout<<"m"<<to_string(world_v_0)<<to_string(world_v_1)<<to_string(world_v_2)<<"\n";
    // return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &projection_transform = *(Square4 *)get_global_data(3);
    Vec4 homo_world_position{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_view = mvdot<float, 4>(view_transform, homo_world_position);
    auto p_in_projection = mvdot<float, 4>(projection_transform, p_in_view);
    new_vertex[0] = p_in_projection[0];
    new_vertex[1] = p_in_projection[1];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    // intepolation world position
    new_vertex.insert(new_vertex.end(), homo_world_position.begin(),
                      homo_world_position.end() - 1);
    // perspective correct
    {
      float w = new_vertex[3];
      for (int i = 0; i < new_vertex.size(); i++) {
        new_vertex[i] /= w;
      }
      new_vertex[3] = 1.0f / w;
    }
    // std::cout<<to_string(new_vertex)<<"\n";
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    Texture &decal_texture = *(Texture *)get_global_data(7);
    DecalCanvasCoordination &decal_canvas_coordination =
        *(DecalCanvasCoordination *)get_global_data(8);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Color texture_color = Color{0.0f, 0.0f, 0.0f};
    Vec2 decal_canvas_coord = Vec2{0.0f, 0.0f};
    if (decal_canvas_coordination.projection(world_position, norm,
                                             decal_canvas_coord)) {
      texture_color =
          decal_texture.sample(decal_canvas_coord[0], decal_canvas_coord[1]);
      if (texture_color[0] == 0.0f && texture_color[1] == 0.0f &&
          texture_color[2] == 0.0f) {
        texture_color = texture.sample(u, v);
      }
    } else {
      texture_color = texture.sample(u, v);
    }
    for (auto &light : lights) {
      Color color =
          light->shading(world_position, norm, camera_world_position) *
          texture_color;

      final_color += color;
    }
    // std::cout<<THIS_LINE<<to_string(final_color)<<"\n";
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, decal_texture_blinn_phong, DecalTextureBlinnPhongShader)

class SDFDecalTextureBlinnPhongShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    Square4 &model_transform = *(Square4 *)get_global_data(0);
    std::vector<Plane> &clipping_planes =
        *(std::vector<Plane> *)get_global_data(1);
    auto world_v_0 = transform_to_world(model_transform, v_0);
    auto world_v_1 = transform_to_world(model_transform, v_1);
    auto world_v_2 = transform_to_world(model_transform, v_2);
    Square3 normal_matrix =
        transpose<float, 3>(inverse3(shrink(model_transform)));
    transform_norm_to_world_inplace(normal_matrix, world_v_0, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_1, 6);
    transform_norm_to_world_inplace(normal_matrix, world_v_2, 6);
    // std::cout<<"m"<<to_string(world_v_0)<<to_string(world_v_1)<<to_string(world_v_2)<<"\n";
    // return std::vector<VertexStageData>{world_v_0, world_v_1, world_v_2};
    return clipping(world_v_0, world_v_1, world_v_2, clipping_planes);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &view_transform = *(Square4 *)get_global_data(2);
    Square4 &projection_transform = *(Square4 *)get_global_data(3);
    Vec4 homo_world_position{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_view = mvdot<float, 4>(view_transform, homo_world_position);
    auto p_in_projection = mvdot<float, 4>(projection_transform, p_in_view);
    new_vertex[0] = p_in_projection[0];
    new_vertex[1] = p_in_projection[1];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    // intepolation world position
    new_vertex.insert(new_vertex.end(), homo_world_position.begin(),
                      homo_world_position.end() - 1);
    // perspective correct
    {
      float w = new_vertex[3];
      for (int i = 0; i < new_vertex.size(); i++) {
        new_vertex[i] /= w;
      }
      new_vertex[3] = 1.0f / w;
    }
    // std::cout<<to_string(new_vertex)<<"\n";
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    std::vector<std::shared_ptr<Light>> &lights =
        *(std::vector<std::shared_ptr<Light>> *)get_global_data(4);
    Vec3 &camera_world_position = *(Vec3 *)get_global_data(5);
    Texture &texture = *(Texture *)get_global_data(6);
    std::function<float(const Vec3 &)> &sdf_func =
        *(std::function<float(const Vec3 &)> *)get_global_data(7);
    Color &sdf_color = *(Color *)get_global_data(8);
    float u = frag.second[4];
    float v = frag.second[5];
    Vec3 norm = Vec3{frag.second[6], frag.second[7], frag.second[8]};
    Vec3 world_position =
        Vec3{frag.second[9], frag.second[10], frag.second[11]};
    Color texture_color = texture.sample(u, v);
    for (auto &light : lights) {
      Color color =
          light->shading(world_position, norm, camera_world_position) *
          texture_color;
      final_color += color;
    }
    if (sin(10.0f * sdf_func(world_position)) > 0) {
      final_color = sdf_color;
    }
    return std::make_tuple(frag.first, final_color, depth);
  }
};
REGISTER_SHADER(Shader, sdf_decal_texture_blinn_phong,
                SDFDecalTextureBlinnPhongShader)

bool convert_screen_space_to_world_space(
    int x_pos, int y_pos, float camera_near, float camera_far,
    const SSAADepthImage &image, const Square4 &inverse_projection_transform,
    const Square4 &inverse_view_transform, Vec3 &world_postion) {
  auto width = image.SSAA_width_ - 1;
  auto height = image.SSAA_height_ - 1;
  auto x = (x_pos / float(width)) * 2.0f - 1.0f;
  auto y = ((height - y_pos) / float(height)) * 2.0f - 1.0f;
  auto z = image.get_depth(x_pos, y_pos);
  if (z == std::numeric_limits<float>::lowest()) {
    return false;
  }
  auto w = (z / (2.0f / (camera_near - camera_far))) +
           0.5f * (camera_near + camera_far);
  // std::cout << x*w << " " << y*w << " " << z << " " << -w << "\n";
  auto view_space_point =
      mvdot<float, 4>(inverse_projection_transform, Vec4{-x * w, -y * w, z, -w});
  // std::cout<<to_string(view_space_point)<<"\n";
  auto world_space_point =
      mvdot<float, 4>(inverse_view_transform, view_space_point);
  // std::cout<<to_string(world_space_point)<<"\n";
  world_postion = shrink(world_space_point);
  return true;
}
class ScreenSpaceDecalDepthTextureBlinnPhongShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    SSAADepthImage &image = *(SSAADepthImage *)get_global_data(0);
    Square4 &inverse_veiw_transform = *(Square4 *)get_global_data(1);
    Square4 &inverse_projection_transform = *(Square4 *)get_global_data(2);
    float camera_near = *(float *)get_global_data(3);
    float camera_far = *(float *)get_global_data(4);
    Texture &decal_texture = *(Texture *)get_global_data(5);
    DecalCanvasCoordination &decal_canvas_coordination =
        *(DecalCanvasCoordination *)get_global_data(6);
    Vec3 world_position{0.0f, 0.0f, 0.0f};
    final_color = image.get(frag.first[0], frag.first[1]);
    if (convert_screen_space_to_world_space(
            frag.first[0], frag.first[1], camera_near, camera_far, image,
            inverse_projection_transform, inverse_veiw_transform,
            world_position)) {
      Vec2 decal_canvas_coord = Vec2{0.0f, 0.0f};
      if (decal_canvas_coordination.projection(world_position,
                                               decal_canvas_coord)) {
        auto decal_color =
            decal_texture.sample(decal_canvas_coord[0], decal_canvas_coord[1]);
        if (decal_color[0] != 0.0f || decal_color[1] != 0.0f ||
            decal_color[2] != 0.0f) {
          final_color = decal_color;
        }
      }
    }

    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, screen_space_decal_depth_texture_blinn_phong,
                ScreenSpaceDecalDepthTextureBlinnPhongShader)
class ScreenSpaceDecalTextureBlinnPhongShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    SSAADepthImage &image = *(SSAADepthImage *)get_global_data(0);
    Square4 &inverse_veiw_transform = *(Square4 *)get_global_data(1);
    Square4 &inverse_projection_transform = *(Square4 *)get_global_data(2);
    float camera_near = *(float *)get_global_data(3);
    float camera_far = *(float *)get_global_data(4);
    Texture &decal_texture = *(Texture *)get_global_data(5);
    DecalCanvasCoordination &decal_canvas_coordination =
        *(DecalCanvasCoordination *)get_global_data(6);
    Vec3 world_position{0.0f, 0.0f, 0.0f};
    final_color = image.get(frag.first[0], frag.first[1]);
    if (convert_screen_space_to_world_space(
            frag.first[0], frag.first[1], camera_near, camera_far, image,
            inverse_projection_transform, inverse_veiw_transform,
            world_position)) {
      Vec2 decal_canvas_coord = Vec2{0.0f, 0.0f};
      // std::cout<<to_string(world_position)<<"\n";
      if (decal_canvas_coordination.projection(world_position,
                                               decal_canvas_coord)) {
        auto decal_color =
            decal_texture.sample(decal_canvas_coord[0], decal_canvas_coord[1]);
        if (decal_color[0] != 0.0f || decal_color[1] != 0.0f ||
            decal_color[2] != 0.0f) {
          final_color = decal_color;
        }
      }
    }

    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, screen_space_decal_texture_blinn_phong,
                ScreenSpaceDecalTextureBlinnPhongShader)

class ScreenSpaceSDFDecalTextureBlinnPhongShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    return std::vector<VertexStageData>{v_0, v_1, v_2};
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    return vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    Color final_color = Color{0.0f, 0.0f, 0.0f};
    SSAADepthImage &image = *(SSAADepthImage *)get_global_data(0);
    Square4 &inverse_veiw_transform = *(Square4 *)get_global_data(1);
    Square4 &inverse_projection_transform = *(Square4 *)get_global_data(2);
    float camera_near = *(float *)get_global_data(3);
    float camera_far = *(float *)get_global_data(4);
    std::function<float(const Vec3 &)> &sdf_func =
        *(std::function<float(const Vec3 &)> *)get_global_data(5);
    Color &sdf_color = *(Color *)get_global_data(6);
    Vec3 world_position{0.0f, 0.0f, 0.0f};
    final_color = image.get(frag.first[0], frag.first[1]);
    if (convert_screen_space_to_world_space(
            frag.first[0], frag.first[1], camera_near, camera_far, image,
            inverse_projection_transform, inverse_veiw_transform,
            world_position)) {
      if (sdf_func(world_position)> 0) {
        final_color = sdf_color;
      }
    }

    return std::make_tuple(frag.first, final_color, 0.0f);
  }
};
REGISTER_SHADER(Shader, screen_space_sdf_decal_texture_blinn_phong,
                ScreenSpaceSDFDecalTextureBlinnPhongShader)