#include <vector>

#include "base_indicator.hpp"
#include "blinn_phong_light_model.hpp"
#include "camera.hpp"
#include "cube.hpp"
#include "deferred_shading.hpp"
#include "depth_image.hpp"
#include "light.hpp"
#include "renderer.hpp"
#include "simple_shader.hpp"
#include "sky_box.hpp"
#include "environment_mapping.hpp"
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
    Model screen_model;
    {
      screen_model.points_ = std::vector<Vec3>{
          Vec3{-1.0f, -1.0f, -0.01f}, Vec3{1.0f, -1.0f, -0.01f},
          Vec3{-1.0f, 1.0f, -0.01f}, Vec3{1.0f, 1.0f, -0.01f}};
      screen_model.triangles_.push_back(std::tuple<int, int, int>(0, 1, 3));
      screen_model.triangles_.push_back(std::tuple<int, int, int>(3, 2, 0));
      screen_model.attributes_.resize(screen_model.triangles_.size());
    }
    model_pool.models_["screen"] = std::move(screen_model);
    model_pool.models_["test"] = std::move(test_model);
    Model sky_cube_box;
    {
      sky_cube_box.points_ = std::vector<Vec3>{
          {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
          {-1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
          {1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, -1.0f}};
      sky_cube_box.triangles_ = std::vector<std::tuple<int, int, int>>{
          {0, 1, 2}, {2, 3, 0}, {5, 4, 7}, {7, 6, 5}, {1, 5, 6}, {6, 2, 1},
          {4, 0, 3}, {3, 7, 4}, {3, 2, 6}, {6, 7, 3}, {1, 0, 4}, {4, 5, 1}};
      sky_cube_box.attributes_.resize(sky_cube_box.triangles_.size());
    }
    model_pool.models_["sky_box_cube"] = std::move(sky_cube_box);
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
                                      SSAADepthImage &image,
                                      const ModelPool &model_pool,
                                      const Instance &instance) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("simple");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void blinn_phong_render_homoposition_color_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void norm_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture, std::shared_ptr<Texture> &norm_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("norm_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)norm_texture.get());
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}

void uv_deferred_geometry_rendering_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera,
    SSAAGBufferDepthImage &g_buffer_image, const ModelPool &model_pool,
    const Instance &instance, std::shared_ptr<Texture> &texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> geometry_shader =
      shader_keeper.build("deferred_geometry");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = geometry_shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)texture.get());
  renderer->shader_->set_global_data(5, (void *)&g_buffer_image);
  renderer->frame_width_ = g_buffer_image.SSAA_width_;
  renderer->frame_height_ = g_buffer_image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->result_bypass_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void uv_deferred_lights_rendering(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool,
    const std::vector<std::shared_ptr<Light>> &lights,
    SSAAGBufferDepthImage &g_buffer_image) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader = shader_keeper.build("deferred_light");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&lights);
  renderer->shader_->set_global_data(1, (void *)&camera.position_);
  renderer->shader_->set_global_data(2, (void *)&g_buffer_image);
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find("screen");
  CHECK(iter != model_pool.models_.end())
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = false;
  renderer->enable_back_face_culling_ = true;
  renderer->result_bypass_ = false;
  renderer->enable_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}

void sky_box_render_homo_view_position(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, std::shared_ptr<Texture> &cube_map_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("sky_box");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(
      0, (void *)&camera.clipping_planes_in_view_space_);
  renderer->shader_->set_global_data(1,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(2, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(3, (void *)cube_map_texture.get());
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find("sky_box_cube");
  CHECK(iter != model_pool.models_.end())
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = false;
  renderer->enable_back_face_culling_ = false;
  renderer->enable_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void environment_mapping_reflection_rendering_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    std::shared_ptr<Texture> &cube_map_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("environment_mapping_reflection");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&camera.position_);
  renderer->shader_->set_global_data(5, (void *)cube_map_texture.get());
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void environment_mapping_refraction_rendering_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    std::shared_ptr<Texture> &cube_map_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("environment_mapping_refraction");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&camera.position_);
  renderer->shader_->set_global_data(5, (void *)cube_map_texture.get());
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find(instance.name_);
  CHECK(iter != model_pool.models_.end())
  auto model_transform = instance.to_model_transform();
  renderer->shader_->set_global_data(0, (void *)&model_transform);
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = instance.is_frame_;
  renderer->enable_back_face_culling_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void build_camera_frustum(ModelPool &model_pool,
                          PerspectiveCameraConfig &config,
                          PerspectiveCamera &camera) {
  Vec3 top_left = config.position_ +
                  camera.coord_base_.bz * config.near_ * 1.01 +
                  camera.coord_base_.bx * config.left_ +
                  camera.coord_base_.by * config.top_;
  Vec3 top_right = config.position_ +
                   camera.coord_base_.bz * config.near_ * 1.01 +
                   camera.coord_base_.bx * config.right_ +
                   camera.coord_base_.by * config.top_;
  Vec3 down_left = config.position_ +
                   camera.coord_base_.bz * config.near_ * 1.01 +
                   camera.coord_base_.bx * config.left_ +
                   camera.coord_base_.by * config.down_;
  Vec3 down_right = config.position_ +
                    camera.coord_base_.bz * config.near_ * 1.01 +
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
      {0, 0, 1},
      {1, 1, 2},
      {2, 2, 3},
      {3, 3, 0},
  };
  // {4, 4, 5}, {5, 5, 6}, {6, 6, 7}, {7, 7, 4}};
  auto colors = std::vector<Color>{
      Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f},
      Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f},
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
    camera_config.position_ = Vec3{5.0f, 0.0f, 5.0f};
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
    size_t SSAA = 2;
    SSAADepthImage image{width, height, SSAA};
    ModelPool model_pool = build_model_pool();
    build_camera_frustum(model_pool, camera_config, camera);
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(make_directional_light(Vec3{-0.5f, -0.5f, -0.5f},
                                            Color{1.0f, 1.0f, 1.0f}));
    lights.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
    lights.push_back(make_point_light(world_centor + Vec3{0.0f, 2.0f, 0.0f},
                                      Color{1.0f, 1.0f, 1.0f}));
    // {
    //   auto base_indicator_instance =
    //       Instance{"base", Vec3{0.0f, 0.0f, 0.0f}, Vec3{50.0f, 50.0f, 50.0f},
    //                eye<float, 3>(), false};
    //   simple_render_homoposition_color(renderer.get(), camera, image,
    //                                    model_pool, base_indicator_instance);
    // }
    // {
    //   auto frustum = Instance{"frustum", Vec3{0.0f, 0.0f, 0.0f},
    //                           Vec3{1.0f, 1.0f, 1.0f}, eye<float, 3>(),
    //                           false};
    //   simple_render_homoposition_color(renderer.get(), camera, image,
    //                                    model_pool, frustum);
    // }
    // {
    //   auto test_triangle_instance_0 =
    //       Instance{"test", Vec3{0.0f, 0.0f, 0.0f}, Vec3{1.0f, 1.0f, 1.0f},
    //                eye<float, 3>(), false};
    //   auto test_triangle_instance_1 =
    //       Instance{"test", Vec3{0.0f, 0.0f, -1.0f}, Vec3{-2.0f, -2.0f, 1.0f},
    //                eye<float, 3>(), false};
    //   simple_render_homoposition_color(renderer.get(), camera, image,
    //                                    model_pool, test_triangle_instance_0);
    //   simple_render_homoposition_color(renderer.get(), camera, image,
    //                                    model_pool, test_triangle_instance_1);
    // }
    // {
    //   auto sphere_instance =
    //       Instance{"sphere", Vec3{0.0f, 0.0f, 0.0f}, Vec3{4.0f, 4.0f, 4.0f},
    //                eye<float, 3>(), false};
    //   blinn_phong_render_homoposition_color_norm(
    //       renderer.get(), camera, image, model_pool, sphere_instance,
    //       lights);
    // }
    {
      std::string base_path = "../../resources/skybox/";
      // face to +z
      std::vector<std::string> paths{
          //+x -x
          base_path + "left.jpg", base_path + "right.jpg",
          //+y -y
          base_path + "top.jpg", base_path + "bottom.jpg",
          //+z -z
          base_path + "front.jpg", base_path + "back.jpg"};
      auto cube_map_texture = make_cube_map_texture(paths);
      {
        sky_box_render_homo_view_position(renderer.get(), camera, image,
                                          model_pool, cube_map_texture);
      }
      {
        Square3 rotate{0.3333333,  -0.2440169, 0.9106836, 0.9106836, 0.3333333,
                       -0.2440169, -0.2440169, 0.9106836, 0.3333333};
        // auto figure_texture = make_check_texture();
        auto uv_cube_instance = Instance{"uv_cube", Vec3{1.6f, 0.0f, 0.0f},
                                         Vec3{2.0f, 2.0f, 2.0f}, rotate, false};
        environment_mapping_reflection_rendering_homoposition_uv_norm(
            renderer.get(), camera, image, model_pool, uv_cube_instance,
            cube_map_texture);
      }
      {
        auto uv_sphere_instance = Instance{"uv_sphere", Vec3{-2.6f, 0.0f, 0.0f},
                                         Vec3{4.0f, 4.0f, 4.0f}, eye<float,3>(), false};
        environment_mapping_refraction_rendering_homoposition_uv_norm(
            renderer.get(), camera, image, model_pool, uv_sphere_instance,
            cube_map_texture);
      }
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
}