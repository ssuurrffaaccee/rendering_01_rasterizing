#include <vector>

#include "base_indicator.hpp"
#include "blinn_phong_light_model.hpp"
#include "camera.hpp"
#include "clipping_geometry_shader.hpp"
#include "cube.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "renderer.hpp"
#include "simple_shader.hpp"
#include "sphere.hpp"
float to_unit(float x) { return (x + 1.0f) / 2.0f; }
ModelPool build_model_pool() {
  // World world{};
  ModelPool model_pool;
  {
    SphereGenerator1 sphere_generator{5};
    Model sphere = sphere_generator.get();
    {
      sphere.attributes_.resize(sphere.triangles_.size());
      Color color = Color{1.0f, 0.0f, 0.0f};
      for (int i = 0; i < sphere.triangles_.size(); i++) {
        auto &[a_idx, b_idx, c_idx] = sphere.triangles_[i];
        append(sphere.attributes_[i], color, color, color);
      }
      for (int i = 0; i < sphere.triangles_.size(); i++) {
        auto &[a_idx, b_idx, c_idx] = sphere.triangles_[i];
        append(sphere.attributes_[i], normalize(sphere.points_[a_idx]),
               normalize(sphere.points_[b_idx]),
               normalize(sphere.points_[c_idx]));
      }
    }
    Model uv_shpere = sphere;
    {
      uv_shpere.attributes_.clear();
      uv_shpere.attributes_.resize(uv_shpere.triangles_.size());
      for (int i = 0; i < uv_shpere.triangles_.size(); i++) {
        auto &[a_idx, b_idx, c_idx] = uv_shpere.triangles_[i];
        auto p_a = normalize(uv_shpere.points_[a_idx]);
        auto p_b = normalize(uv_shpere.points_[b_idx]);
        auto p_c = normalize(uv_shpere.points_[c_idx]);
        // std::cout<<to_string(Vec2{to_unit(p_a[0]),
        // to_unit(p_a[1])})<<to_string(Vec2{to_unit(p_b[0]),
        // to_unit(p_b[1])})<<to_string(Vec2{to_unit(p_c[0]),
        // to_unit(p_c[1])})<<"\n";
        // append uv
        append(uv_shpere.attributes_[i], Vec2{to_unit(p_a[0]), to_unit(p_a[1])},
               Vec2{to_unit(p_b[0]), to_unit(p_b[1])},
               Vec2{to_unit(p_c[0]), to_unit(p_c[1])});
        // append norm
        append(uv_shpere.attributes_[i], p_a, p_b, p_c);
      }
    }
    Model cube = get_cube();
    model_pool.models_["cube"] = std::move(cube);
    Model base_indicator = get_base_indicator();
    model_pool.models_["base"] = std::move(base_indicator);
    model_pool.models_["sphere"] = std::move(sphere);
    model_pool.models_["uv_sphere"] = std::move(uv_shpere);
    model_pool.models_["uv_cube"] = get_uv_cube();
    Model test_model;
    {
      test_model.points_ =
          std::vector<Vec3>{Vec3{0.0f, 1.0f, -0.01f}, Vec3{-1.0f, 0.0f, -0.01f},
                            Vec3{1.0f, 0.0f, -0.01f}};
      auto colors =
          std::vector<Color>{Color{1.0f, 0.0f, 0.0f}, Color{0.0f, 1.0f, 0.0f},
                             Color{0.0f, 0.0f, 1.0f}};
      test_model.triangles_.push_back(std::tuple<int, int, int>(0, 1, 2));
      test_model.attributes_.resize(test_model.triangles_.size());
      for (int i = 0; i < test_model.triangles_.size(); i++) {
        auto &[a_idx, b_idx, c_idx] = test_model.triangles_[i];
        append(test_model.attributes_[i], colors[a_idx], colors[b_idx],
               colors[c_idx]);
      }
    }
    model_pool.models_["test"] = std::move(test_model);
  }
  return model_pool;
}
std::vector<VertexStageData> model_to_vertices(const Model &model) {
  std::vector<VertexStageData> vertices;
  for (int i = 0; i < model.points_.size(); i++) {
    VertexStageData data;
    // homo_position
    data.insert(data.end(), model.points_[i].begin(), model.points_[i].end());
    data.push_back(1.0f);
    vertices.push_back(std::move(data));
  }
  return vertices;
}
void simple_render_homoposition_color(Renderer *renderer,
                                      const PerspectiveCamera &camera,
                                      DepthImage &image, int width, int height,
                                      const ModelPool &model_pool,
                                      const Instance &instance) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<VertexShader> vertex_shader =
      shader_keeper.build_vertex("simple");
  std::shared_ptr<FragmentShader> fragment_shader =
      shader_keeper.build_fragment("simple");
  std::shared_ptr<GeometryShader> gemoetry_shader =
      shader_keeper.build_geometry("clipping_triangle");
  renderer->geometry_shader_ = gemoetry_shader.get();
  renderer->fragment_shader_ = fragment_shader.get();
  renderer->vertex_shader_ = vertex_shader.get();
  renderer->vertex_shader_->set_global_data(
      0, (void *)&camera.world_to_view_transform_);
  renderer->vertex_shader_->set_global_data(
      1, (void *)&camera.projection_transform_);
  renderer->geometry_shader_->set_global_data(1,
                                              (void *)&camera.clipping_planes_);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = width;
  renderer->frame_height_ = height;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->geometry_shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void blinn_phong_render_homoposition_color_norm(
    Renderer *renderer, const PerspectiveCamera &camera, DepthImage &image,
    int width, int height, const ModelPool &model_pool,
    const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<VertexShader> vertex_shader =
      shader_keeper.build_vertex("blinn_phong");
  std::shared_ptr<FragmentShader> fragment_shader =
      shader_keeper.build_fragment("blinn_phong");
  std::shared_ptr<GeometryShader> gemoetry_shader =
      shader_keeper.build_geometry("clipping_triangle_7");
  renderer->geometry_shader_ = gemoetry_shader.get();
  renderer->fragment_shader_ = fragment_shader.get();
  renderer->vertex_shader_ = vertex_shader.get();
  renderer->fragment_shader_->set_global_data(0, (void *)&lights);
  renderer->fragment_shader_->set_global_data(1, (void *)&camera.position_);
  renderer->vertex_shader_->set_global_data(
      0, (void *)&camera.world_to_view_transform_);
  renderer->vertex_shader_->set_global_data(
      1, (void *)&camera.projection_transform_);
  renderer->geometry_shader_->set_global_data(1,
                                              (void *)&camera.clipping_planes_);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = width;
  renderer->frame_height_ = height;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->geometry_shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, DepthImage &image,
    int width, int height, const ModelPool &model_pool,
    const Instance &instance, const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<VertexShader> vertex_shader =
      shader_keeper.build_vertex("blinn_phong");
  std::shared_ptr<FragmentShader> fragment_shader =
      shader_keeper.build_fragment("uv_blinn_phong");
  std::shared_ptr<GeometryShader> gemoetry_shader =
      shader_keeper.build_geometry("clipping_triangle_6");
  renderer->geometry_shader_ = gemoetry_shader.get();
  renderer->fragment_shader_ = fragment_shader.get();
  renderer->vertex_shader_ = vertex_shader.get();
  renderer->fragment_shader_->set_global_data(0, (void *)&lights);
  renderer->fragment_shader_->set_global_data(1, (void *)&camera.position_);
  renderer->fragment_shader_->set_global_data(2, (void *)texture.get());
  renderer->vertex_shader_->set_global_data(
      0, (void *)&camera.world_to_view_transform_);
  renderer->vertex_shader_->set_global_data(
      1, (void *)&camera.projection_transform_);
  renderer->geometry_shader_->set_global_data(1,
                                              (void *)&camera.clipping_planes_);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = width;
  renderer->frame_height_ = height;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->geometry_shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void build_camera_frustum(ModelPool &model_pool,
                          PerspectiveCameraConfig &config,
                          PerspectiveCamera &camera) {
  Vec3 top_left = config.position_ + camera.coord_base_.bz * config.near_*1.01 +
                  camera.coord_base_.bx * config.left_ +
                  camera.coord_base_.by * config.top_;
  Vec3 top_right = config.position_ + camera.coord_base_.bz * config.near_*1.01 +
                   camera.coord_base_.bx * config.right_ +
                   camera.coord_base_.by * config.top_;
  Vec3 down_left = config.position_ + camera.coord_base_.bz * config.near_*1.01 +
                   camera.coord_base_.bx * config.left_ +
                   camera.coord_base_.by * config.down_;
  Vec3 down_right = config.position_ + camera.coord_base_.bz * config.near_*1.01 +
                    camera.coord_base_.bx * config.right_ +
                    camera.coord_base_.by * config.down_;
  Vec3 far_top_left =
      config.position_ +
      normalize(top_left - config.position_) * std::abs(config.far_);
  Vec3 far_top_right =
      config.position_ +
      normalize(top_right - config.position_) * std::abs(config.far_);
  Vec3 far_down_left =
      config.position_ +
      normalize(down_left - config.position_) * std::abs(config.far_);
  Vec3 far_down_right =
      config.position_ +
      normalize(down_right - config.position_) * std::abs(config.far_);
  Model model;
  model.points_ = std::vector<Vec3>{
      down_left,     down_right,     top_left,     top_right,
      far_down_left, far_down_right, far_top_left, far_top_right,
  };
  model.triangles_ = std::vector<std::tuple<int, int, int>>{
      {0, 0, 1}, {1, 1, 2}, {2, 2, 3}, {3, 3, 0},};
      // {4, 4, 5}, {5, 5, 6}, {6, 6, 7}, {7, 7, 4}};
  auto colors = std::vector<Color>{
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f},
  };
  model.attributes_.resize(model.triangles_.size());
  for (int i = 0; i < model.triangles_.size(); i++) {
    auto &[a_idx, b_idx, c_idx] = model.triangles_[i];
    append(model.attributes_[i], colors[a_idx], colors[b_idx], colors[c_idx]);
  }
  model_pool.models_["frustum"] = model;
}
int main() {
  try {
    Vec3 world_centor = Vec3{0.0f, 0.0f, 0.0f};
    size_t height = 1000;
    size_t width = 1000;
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{0.0f, 0.0f, 4.0f};
    camera_config.forward_ = world_centor - camera_config.position_;
    camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
    camera_config.far_ = -1000.0f;
    camera_config.near_ = -1.0f;
    camera_config.left_ = -1.0f;
    camera_config.right_ = 1.0f;
    camera_config.top_ = 1.0f;
    camera_config.down_ = -1.0f;
    PerspectiveCamera camera;
    camera.init(camera_config);
    DepthImage image{width, height};
    ModelPool model_pool = build_model_pool();
    build_camera_frustum(model_pool, camera_config, camera);
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(make_directional_light(Vec3{-0.5f, -0.5f, -0.5f},
                                            Color{1.0f, 1.0f, 1.0f}));
    lights.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
    lights.push_back(make_point_light(world_centor + Vec3{0.0f, 2.0f, 0.0f},
                                      Color{1.0f, 1.0f, 1.0f}));
    {
      auto base_indicator_instance =
          Instance{"base", Vec3{0.0f, 0.0f, 0.0f}, Vec3{50.0f, 50.0f, 50.0f},
                   eye<float, 3>(), false};
      simple_render_homoposition_color(renderer.get(), camera, image, width,
                                       height, model_pool,
                                       base_indicator_instance);
    }
    // {
    //   auto frustum = Instance{"frustum", Vec3{0.0f, 0.0f, 0.0f},
    //                           Vec3{1.0f, 1.0f, 1.0f}, eye<float, 3>(), false};
    //   simple_render_homoposition_color(renderer.get(), camera, image, width,
    //                                    height, model_pool, frustum);
    // }
    // {
    //   auto test_triangle_instance_0 =
    //       Instance{"test", Vec3{0.0f, 0.0f, 0.0f}, Vec3{1.0f, 1.0f, 1.0f},
    //                eye<float, 3>(), true};
    //   auto test_triangle_instance_1 =
    //       Instance{"test", Vec3{0.0f, 0.0f, -1.0f}, Vec3{-2.0f, -2.0f, 1.0f},
    //                eye<float, 3>(), true};
    //   simple_render_homoposition_color(renderer.get(), camera, image, width,
    //                                    height, model_pool,
    //                                    test_triangle_instance_0);
    //   simple_render_homoposition_color(renderer.get(), camera, image, width,
    //                                    height, model_pool,
    //                                    test_triangle_instance_1);
    // }
    // {
    //   auto sphere_instance =
    //       Instance{"sphere", Vec3{0.0f, 0.0f, 0.0f}, Vec3{2.0f, 2.0f, 2.0f},
    //                eye<float, 3>(), false};
    //   blinn_phong_render_homoposition_color_norm(renderer.get(), camera,
    //   image,
    //                                              width, height, model_pool,
    //                                              sphere_instance, lights);
    // }
    {
      auto uv_sphere_instance =
          Instance{"uv_sphere", Vec3{-1.7f, 0.0f, 0.0f},
          Vec3{2.8f, 2.8f, 2.8f},
                   eye<float, 3>(), false};
      // auto texture = make_check_texture();
      auto texture = make_color_texture(Color{1.0f,0.0f,0.0f});
      uv_blinn_phong_renader_homoposition_uv_norm(
          renderer.get(), camera, image, width, height, model_pool,
          uv_sphere_instance, lights, texture);
    }
    {
      Square3 rotate{0.3333333,  -0.2440169, 0.9106836, 0.9106836, 0.3333333,
                     -0.2440169, -0.2440169, 0.9106836, 0.3333333};
      auto figure_texture = make_image_texture("../crate-texture.jpg");
      // auto figure_texture = make_check_texture();
      auto uv_cube_instance =
          Instance{"uv_cube", Vec3{1.9f, 0.0f, 0.0f}, Vec3{1.0f, 1.0f, 1.0f},
                   rotate, false};
      uv_blinn_phong_renader_homoposition_uv_norm(
          renderer.get(), camera, image, width, height, model_pool,
          uv_cube_instance, lights, figure_texture);
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
}