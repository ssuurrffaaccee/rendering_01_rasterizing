#include <vector>

#include "base_indicator.hpp"
#include "blinn_phong_light_model.hpp"
#include "bloom_and_hdr.hpp"
#include "camera.hpp"
#include "cube.hpp"
#include "decal.hpp"
#include "deferred_shading.hpp"
#include "depth_image.hpp"
#include "environment_mapping.hpp"
#include "ibl.hpp"
#include "light.hpp"
#include "pbr.hpp"
#include "pbr_light.hpp"
#include "point_shadow_mapping.hpp"
#include "post_processing.hpp"
#include "renderer.hpp"
#include "shadow_mapping.hpp"
#include "simple_shader.hpp"
#include "sky_box.hpp"
#include "sphere.hpp"
#include "ssao.hpp"
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
    model_pool.models_["uv_inner_cube"] = get_inner_uv_cube();
    model_pool.models_["uv_bloom_cube"] = get_bloom_uv_cube();
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
    Model uv_screen_match_texture_coordination_model;
    {
      uv_screen_match_texture_coordination_model.points_ = std::vector<Vec3>{
          Vec3{-1.0f, 1.0f, -0.01f}, Vec3{1.0f, 1.0f, -0.01f},
          Vec3{-1.0f, -1.0f, -0.01f}, Vec3{1.0f, -1.0f, -0.01f}};
      uv_screen_match_texture_coordination_model.triangles_.push_back(
          std::tuple<int, int, int>(0, 2, 1));
      uv_screen_match_texture_coordination_model.triangles_.push_back(
          std::tuple<int, int, int>(1, 2, 3));
      uv_screen_match_texture_coordination_model.attributes_.resize(
          uv_screen_match_texture_coordination_model.triangles_.size());
      auto uv_0 = Vec2{0.0f, 0.0f};
      auto uv_1 = Vec2{1.0f, 0.0f};
      auto uv_2 = Vec2{0.0f, 1.0f};
      auto uv_3 = Vec2{1.0f, 1.0f};
      append(uv_screen_match_texture_coordination_model.attributes_[0], uv_0,
             uv_2, uv_1);
      append(uv_screen_match_texture_coordination_model.attributes_[1], uv_1,
             uv_2, uv_3);
      auto norm = Vec3{0.0f, 0.0f, 1.0f};
      append(uv_screen_match_texture_coordination_model.attributes_[0], norm,
             norm, norm);
      append(uv_screen_match_texture_coordination_model.attributes_[1], norm,
             norm, norm);
    }
    model_pool.models_["uv_screen_match_texture_coordination"] =
        std::move(uv_screen_match_texture_coordination_model);
    Model uv_screen_model;
    {
      uv_screen_model.points_ = std::vector<Vec3>{
          Vec3{-1.0f, -1.0f, -0.01f}, Vec3{1.0f, -1.0f, -0.01f},
          Vec3{-1.0f, 1.0f, -0.01f}, Vec3{1.0f, 1.0f, -0.01f}};
      uv_screen_model.triangles_.push_back(std::tuple<int, int, int>(0, 1, 3));
      uv_screen_model.triangles_.push_back(std::tuple<int, int, int>(3, 2, 0));
      uv_screen_model.attributes_.resize(uv_screen_model.triangles_.size());
      auto uv_0 = Vec2{0.0f, 0.0f};
      auto uv_1 = Vec2{0.0f, 1.0f};
      auto uv_2 = Vec2{1.0f, 0.0f};
      auto uv_3 = Vec2{1.0f, 1.0f};
      append(uv_screen_model.attributes_[0], uv_0, uv_1, uv_3);
      append(uv_screen_model.attributes_[1], uv_3, uv_2, uv_0);
      auto norm = Vec3{0.0f, 0.0f, 1.0f};
      append(uv_screen_model.attributes_[0], norm, norm, norm);
      append(uv_screen_model.attributes_[1], norm, norm, norm);
    }
    model_pool.models_["uv_screen"] = std::move(uv_screen_model);
    model_pool.models_["test"] = std::move(test_model);
    Model uv_point_model;
    {
      float x = 1.0f;
      float y = 1.0f;
      uv_point_model.points_ =
          std::vector<Vec3>{Vec3{x, y, -0.01f}, Vec3{x, y, -0.01f},
                            Vec3{x, y, -0.01f}, Vec3{x, y, -0.01f}};
      uv_point_model.triangles_.push_back(std::tuple<int, int, int>(0, 1, 3));
      uv_point_model.triangles_.push_back(std::tuple<int, int, int>(3, 2, 0));
      uv_point_model.attributes_.resize(uv_point_model.triangles_.size());
      auto uv_0 = Vec2{0.0f, 0.0f};
      auto uv_1 = Vec2{0.0f, 1.0f};
      auto uv_2 = Vec2{1.0f, 0.0f};
      auto uv_3 = Vec2{1.0f, 1.0f};
      append(uv_point_model.attributes_[0], uv_0, uv_1, uv_3);
      append(uv_point_model.attributes_[1], uv_3, uv_2, uv_0);
      auto norm = Vec3{0.0f, 0.0f, 1.0f};
      append(uv_point_model.attributes_[0], norm, norm, norm);
      append(uv_point_model.attributes_[1], norm, norm, norm);
    }
    model_pool.models_["uv_point"] = std::move(uv_point_model);
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
void uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const OrthogonalCamera &camera, SSAADepthImage &image,
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
    const ModelPool &model_pool, std::shared_ptr<Texture> &cube_map_texture,
    bool is_enable_depth_test) {
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
  renderer->enable_depth_test_ = is_enable_depth_test;
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
void shadow_mapping_depth_map_rendering_homoposition(
    Renderer *renderer, const OrthogonalCamera &camera,
    SSAAOnlyDepthImage &image, const ModelPool &model_pool,
    const Instance &instance) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("shadow_mapping_depth_map");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  // renderer->color_buffer_ = image.data_->data();
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
  renderer->enable_only_depth_test_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
  renderer->enable_only_depth_test_ = false;
}
void shadow_mapping_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::shared_ptr<DirectionalLight> &directional_light,
    std::shared_ptr<Texture> &texture, const SSAAOnlyDepthImage &depth_map,
    const Square4 &view_and_projection_transform_with_direction_light) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("shadow_mapping_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&directional_light);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(
      7, (void *)&view_and_projection_transform_with_direction_light);
  renderer->shader_->set_global_data(8, (void *)&depth_map);
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
  renderer->enable_only_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void point_shadow_mapping_depth_map_rendering_homoposition(
    Renderer *renderer, const PerspectiveCamera &camera,
    SSAAOnlyDepthImage &image, const ModelPool &model_pool,
    const Instance &instance) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("point_shadow_mapping_depth_map");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  // renderer->color_buffer_ = image.data_->data();
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
  renderer->enable_only_depth_test_ = true;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
  renderer->enable_only_depth_test_ = false;
}
void point_shadow_mapping_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const PointLight &point_light, std::shared_ptr<Texture> &texture,
    const OnlyDepthCubaImage &cube_depth_map,
    const CubeDepthCalculator &cube_depth_calculator) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("point_shadow_mapping_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&point_light);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)&cube_depth_calculator);
  renderer->shader_->set_global_data(8, (void *)&cube_depth_map);
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
  renderer->enable_only_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void bloom_generate_light_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::shared_ptr<Light> &point_light,
    std::shared_ptr<Texture> &texture, SSAADepthImage &bright_image) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("bloom_generate_light_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&point_light);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)&bright_image);
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
void bloom_generate_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture, SSAADepthImage &bright_image) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("bloom_generate_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)&bright_image);
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
void bloom_gauss_blur(Renderer *renderer, SSAADepthImage &image,
                      const ModelPool &model_pool, SSAADepthImage &bright_image,
                      bool is_horizontal) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("bloom_gauss_blur");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&bright_image);
  renderer->shader_->set_global_data(1, (void *)&is_horizontal);
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
  renderer->enable_depth_test_ = true;
}
void bloom_combine(Renderer *renderer, SSAADepthImage &image,
                   const ModelPool &model_pool, SSAADepthImage &color_image,
                   SSAADepthImage &bright_image, float exposure) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("bloom_combine");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&bright_image);
  renderer->shader_->set_global_data(1, (void *)&color_image);
  renderer->shader_->set_global_data(2, (void *)&exposure);
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
  renderer->enable_depth_test_ = true;
}
void simple_post_processing(Renderer *renderer, SSAADepthImage &image,
                            const ModelPool &model_pool,
                            SSAADepthImage &source_iamge,
                            const std::string &shader_name) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build(shader_name);
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&source_iamge);
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
  renderer->enable_depth_test_ = true;
}
void kernel_post_processing(Renderer *renderer, SSAADepthImage &image,
                            const ModelPool &model_pool,
                            SSAADepthImage &source_iamge,
                            const std::vector<float> &kernel) {
  CHECK(kernel.size() == 9)
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("kernel_post_processing");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&source_iamge);
  renderer->shader_->set_global_data(1, (void *)&kernel);
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
  renderer->enable_depth_test_ = true;
}
void decal_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture, std::shared_ptr<Texture> &decal_texture,
    const DecalCanvasCoordination &decal_canvas_coordination) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("decal_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)decal_texture.get());
  renderer->shader_->set_global_data(8, (void *)&decal_canvas_coordination);
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
void sdf_decal_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture,
    const std::function<float(const Vec3 &)> &sdf_func,
    const Color &sdf_color) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("sdf_decal_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)&sdf_func);
  renderer->shader_->set_global_data(8, (void *)&sdf_color);
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
void screen_space_decal_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, SSAADepthImage &source_iamge,
    const DecalCanvasCoordination &decal_canvas_coordination, float camera_near,
    float camera_far, std::shared_ptr<Texture> &texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("screen_space_decal_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&source_iamge);
  auto inverse_view_transform = inverse4(camera.world_to_view_transform_);
  renderer->shader_->set_global_data(1, (void *)&inverse_view_transform);
  auto inverse_projection_transform = inverse4(camera.projection_transform_);
  renderer->shader_->set_global_data(2, (void *)&inverse_projection_transform);
  renderer->shader_->set_global_data(3, (void *)&camera_near);
  renderer->shader_->set_global_data(4, (void *)&camera_far);
  renderer->shader_->set_global_data(5, (void *)texture.get());
  renderer->shader_->set_global_data(6, (void *)&decal_canvas_coordination);
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
  renderer->enable_depth_test_ = true;
}
void screen_space_sdf_decal_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, SSAADepthImage &source_iamge,
    float camera_near, float camera_far,
    const std::function<float(const Vec3 &)> &sdf_func,
    const Color &sdf_color) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("screen_space_sdf_decal_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&source_iamge);
  auto inverse_view_transform = inverse4(camera.world_to_view_transform_);
  renderer->shader_->set_global_data(1, (void *)&inverse_view_transform);
  auto inverse_projection_transform = inverse4(camera.projection_transform_);
  renderer->shader_->set_global_data(2, (void *)&inverse_projection_transform);
  renderer->shader_->set_global_data(3, (void *)&camera_near);
  renderer->shader_->set_global_data(4, (void *)&camera_far);
  renderer->shader_->set_global_data(5, (void *)&sdf_func);
  renderer->shader_->set_global_data(6, (void *)&sdf_color);
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
  renderer->enable_depth_test_ = true;
}
void parallax_mapping_norm_uv_blinn_phong_renader_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    const std::vector<std::shared_ptr<Light>> &lights,
    std::shared_ptr<Texture> &texture, std::shared_ptr<Texture> &norm_texture,
    std::shared_ptr<Texture> &displacement_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("parallax_mapping_norm_texture_blinn_phong");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);

  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&lights);
  renderer->shader_->set_global_data(5, (void *)&camera.position_);
  renderer->shader_->set_global_data(6, (void *)texture.get());
  renderer->shader_->set_global_data(7, (void *)norm_texture.get());
  renderer->shader_->set_global_data(11, (void *)displacement_texture.get());
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
void ssao_rendering(Renderer *renderer, const PerspectiveCamera &camera,
                    SSAADepthImage &image, const ModelPool &model_pool,
                    SSAAGBufferDepthImage &g_buffer_image,
                    std::shared_ptr<Texture> &noise_texture,
                    std::vector<Vec3> &kernel_smpales, float camera_near,
                    float camera_far) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader = shader_keeper.build("ssao");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(1, (void *)noise_texture.get());
  renderer->shader_->set_global_data(2, (void *)&kernel_smpales);
  renderer->shader_->set_global_data(3,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(4, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(5, (void *)&camera_near);
  renderer->shader_->set_global_data(6, (void *)&camera_far);
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
  renderer->enable_depth_test_ = true;
}
void ssao_blur(Renderer *renderer, SSAADepthImage &image,
               const ModelPool &model_pool, SSAADepthImage &ssao_iamge) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader = shader_keeper.build("ssao_blur");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(0, (void *)&ssao_iamge);
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
  renderer->enable_depth_test_ = true;
}
void ssao_uv_deferred_lights_rendering(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool,
    const std::vector<std::shared_ptr<Light>> &lights,
    SSAAGBufferDepthImage &g_buffer_image, SSAADepthImage &blured_ssao_image) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader =
      shader_keeper.build("ssao_deferred_light");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&lights);
  renderer->shader_->set_global_data(1, (void *)&camera.position_);
  renderer->shader_->set_global_data(2, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(3, (void *)&blured_ssao_image);
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
void pbr_directed_light_uv_deferred_geometry_rendering_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera,
    SSAAGBufferDepthImage &g_buffer_image, const ModelPool &model_pool,
    const Instance &instance, const Color &albedo, float metallic,
    float roughness, float ao) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> geometry_shader =
      shader_keeper.build("pbr_directed_light_deferred_geometry");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = geometry_shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(5, (void *)&albedo);
  renderer->shader_->set_global_data(6, (void *)&metallic);
  renderer->shader_->set_global_data(7, (void *)&roughness);
  renderer->shader_->set_global_data(8, (void *)&ao);
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
void pbr_uv_deferred_lights_rendering(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool,
    const std::vector<std::shared_ptr<PBRLight>> &pbr_lights,
    SSAAGBufferDepthImage &g_buffer_image) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader =
      shader_keeper.build("pbr_deferred_light");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&pbr_lights);
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
  renderer->enable_depth_test_ = true;
}
void pbr_texture_uv_deferred_geometry_rendering_homoposition_uv_norm(
    Renderer *renderer, const PerspectiveCamera &camera,
    SSAAGBufferDepthImage &g_buffer_image, const ModelPool &model_pool,
    const Instance &instance, std::shared_ptr<Texture> &albedo,
    std::shared_ptr<Texture> &metallic, std::shared_ptr<Texture> &roughness,
    std::shared_ptr<Texture> &ao, std::shared_ptr<Texture> &norm) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> geometry_shader =
      shader_keeper.build("pbr_texture_deferred_geometry");
  // std::shared_ptr<Shader> deferred_shader =
  // shader_keeper.build("deferred_light");
  renderer->shader_ = geometry_shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(5, (void *)albedo.get());
  renderer->shader_->set_global_data(6, (void *)metallic.get());
  renderer->shader_->set_global_data(7, (void *)roughness.get());
  renderer->shader_->set_global_data(8, (void *)ao.get());
  renderer->shader_->set_global_data(9, (void *)norm.get());
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
void ibl_equidistant_cylindrical_to_cube_mapping(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    std::shared_ptr<Texture> &equi_rectangular_map) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("ibl_equidistant_cylindrical_to_cube_mapping");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)equi_rectangular_map.get());
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
  renderer->enable_only_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
  renderer->enable_only_depth_test_ = true;
}
void ibl_irradiance_convolution_precompution(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    std::shared_ptr<Texture> &cube_map_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("ibl_irradiance_convolution_precompution");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)cube_map_texture.get());
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
  renderer->enable_only_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void ibl_diffuse_pbr_uv_deferred_lights_rendering(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool,
    const std::vector<std::shared_ptr<PBRLight>> &pbr_lights,
    SSAAGBufferDepthImage &g_buffer_image,
    std::shared_ptr<Texture> &ibl_diffuse_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader =
      shader_keeper.build("ibl_diffuse_pbr_deferred_light");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&pbr_lights);
  renderer->shader_->set_global_data(1, (void *)&camera.position_);
  renderer->shader_->set_global_data(2, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(3, (void *)ibl_diffuse_texture.get());
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
  renderer->enable_depth_test_ = true;
}
void ibl_prefilter_precompution(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool, const Instance &instance,
    std::shared_ptr<Texture> &cube_map_texture, float roughness) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> shader =
      shader_keeper.build("ibl_specular_prefilter_map_computation");
  renderer->shader_ = shader.get();
  renderer->shader_->set_global_data(1, (void *)&camera.clipping_planes_);
  renderer->shader_->set_global_data(2,
                                     (void *)&camera.world_to_view_transform_);
  renderer->shader_->set_global_data(3, (void *)&camera.projection_transform_);
  renderer->shader_->set_global_data(4, (void *)cube_map_texture.get());
  renderer->shader_->set_global_data(5, (void *)&roughness);
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
  renderer->enable_only_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
}
void ibl_specular_brdf_compution(Renderer *renderer, SSAADepthImage &image,
                                 const ModelPool &model_pool) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader =
      shader_keeper.build("ibl_specular_brdf_compution");
  renderer->shader_ = light_shader.get();
  renderer->color_buffer_ = image.data_->data();
  renderer->depth_buffer_ = image.depth_buffer_->data();
  renderer->frame_width_ = image.SSAA_width_;
  renderer->frame_height_ = image.SSAA_height_;
  auto iter = model_pool.models_.find("uv_screen_match_texture_coordination");
  CHECK(iter != model_pool.models_.end())
  auto vertices = model_to_vertices(iter->second);
  renderer->is_draw_framework_ = false;
  renderer->enable_back_face_culling_ = true;
  renderer->result_bypass_ = false;
  renderer->enable_depth_test_ = false;
  renderer->render(vertices, iter->second.triangles_, iter->second.attributes_);
  renderer->enable_depth_test_ = true;
}
void ibl_diffuse_specular_pbr_uv_deferred_lights_rendering(
    Renderer *renderer, const PerspectiveCamera &camera, SSAADepthImage &image,
    const ModelPool &model_pool,
    const std::vector<std::shared_ptr<PBRLight>> &pbr_lights,
    SSAAGBufferDepthImage &g_buffer_image,
    std::shared_ptr<Texture> &ibl_diffuse_texture,
    std::shared_ptr<Texture> &ibl_prefilter_mip_cube_texture,
    std::shared_ptr<Texture> &ibl_brdf_lut_texture) {
  auto &shader_keeper = ShaderKeeper::get();
  std::shared_ptr<Shader> light_shader =
      shader_keeper.build("ibl_diffuse_specular_pbr_deferred_light");
  renderer->shader_ = light_shader.get();
  renderer->shader_->set_global_data(0, (void *)&pbr_lights);
  renderer->shader_->set_global_data(1, (void *)&camera.position_);
  renderer->shader_->set_global_data(2, (void *)&g_buffer_image);
  renderer->shader_->set_global_data(3, (void *)ibl_diffuse_texture.get());
  renderer->shader_->set_global_data(
      4, (void *)ibl_prefilter_mip_cube_texture.get());
  renderer->shader_->set_global_data(5, (void *)ibl_brdf_lut_texture.get());
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
  renderer->enable_depth_test_ = true;
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
std::shared_ptr<Texture> convert_hdr_equidistant_cylindrical_to_cube_mapping(
    Renderer *renderer, const ModelPool &model_pool, int width, int height) {
  auto room_instance =
      Instance{"uv_inner_cube", Vec3{0.0f, 0.0f, 0.0f},
               Vec3{50.0f, 50.0f, 50.0f}, eye<float, 3>(), false};
  std::vector<std::unique_ptr<SSAADepthImage>> hdr_cube_mapping_images;
  auto equi_rectangular_map = make_equidistant_cylindrical_hdr_image_texture(
      "../../resources/textures/hdr/newport_loft.hdr");
  std::vector<Square4> view_and_projection_transforms;
  view_and_projection_transforms.resize(6);
  for (int i = 0; i < 6; i++) {
    hdr_cube_mapping_images.push_back(
        std::make_unique<SSAADepthImage>(width, height, 1));
  }
  std::vector<Vec3> camera_forward_directions{
      //+x -x // left right
      Vec3{1.0f, 0.0f, 0.0f},
      Vec3{-1.0f, 0.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, -1.0f, 0.0f},
      //+z -z // front back
      Vec3{0.0f, 0.0f, 1.0f},
      Vec3{0.0f, 0.0f, -1.0f},
  };
  std::vector<Vec3> camera_up_directions{
      //+x -x // left right
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 0.0f, -1.0f},
      Vec3{0.0f, 0.0f, 1.0f},
      //+z -z // front back
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
  };
  std::vector<std::string> depth_image_names{
      "left",  "right",  //
      "top",   "bottom", //
      "front", "back",   //
  };
  for (int i = 0; i < 6; i++) {
    std::cout << "convert hdr to cube_maping: " << i << "\n";
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{0.0f, 0.0f, 0.0f};
    camera_config.forward_ = camera_forward_directions[i];
    camera_config.up_ = camera_up_directions[i];
    camera_config.far_ = -100.0f;
    camera_config.near_ = -1.0f;
    camera_config.left_ = -1.0f;
    camera_config.right_ = 1.0f;
    camera_config.top_ = 1.0f;
    camera_config.down_ = -1.0f;
    PerspectiveCamera camera;
    camera.init(camera_config);
    view_and_projection_transforms[i] = mmdot<float, 4>(
        camera.projection_transform_, camera.world_to_view_transform_);
    {
      auto &hdr_texture = *(hdr_cube_mapping_images[i].get());
      ibl_equidistant_cylindrical_to_cube_mapping(renderer, camera, hdr_texture,
                                                  model_pool, room_instance,
                                                  equi_rectangular_map);
      // hdr_texture.dump(depth_image_names[i] + "_cube");
    }
  }
  auto cube_map_texture =
      six_image_to_cub_mapping_texture(hdr_cube_mapping_images);
  return cube_map_texture;
}
std::shared_ptr<Texture> compute_irradiance_convolution_texture(
    Renderer *renderer, const ModelPool &model_pool,
    std::shared_ptr<Texture> &cube_map_texture) {
  auto room_instance =
      Instance{"uv_inner_cube", Vec3{0.0f, 0.0f, 0.0f},
               Vec3{50.0f, 50.0f, 50.0f}, eye<float, 3>(), false};
  std::vector<Square4> view_and_projection_transforms;
  view_and_projection_transforms.resize(6);
  std::vector<Vec3> camera_forward_directions{
      //+x -x // left right
      Vec3{1.0f, 0.0f, 0.0f},
      Vec3{-1.0f, 0.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, -1.0f, 0.0f},
      //+z -z // front back
      Vec3{0.0f, 0.0f, 1.0f},
      Vec3{0.0f, 0.0f, -1.0f},
  };
  std::vector<Vec3> camera_up_directions{
      //+x -x // left right
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 0.0f, -1.0f},
      Vec3{0.0f, 0.0f, 1.0f},
      //+z -z // front back
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
  };
  std::vector<std::string> depth_image_names{
      "left",  "right",  //
      "top",   "bottom", //
      "front", "back",   //
  };
  std::vector<std::unique_ptr<SSAADepthImage>> difuse_images;
  // Make it small, else dramatically ssssslllllloooowwww;
  size_t irradiance_size_width{64};
  size_t irradiance_size_height{64};
  for (int i = 0; i < 6; i++) {
    difuse_images.push_back(std::make_unique<SSAADepthImage>(
        irradiance_size_width, irradiance_size_height, 1));
  }
  for (int i = 0; i < 6; i++) {
    std::cout << "irradiance convolution: " << i << "\n";
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{0.0f, 0.0f, 0.0f};
    camera_config.forward_ = camera_forward_directions[i];
    camera_config.up_ = camera_up_directions[i];
    camera_config.far_ = -100.0f;
    camera_config.near_ = -1.0f;
    camera_config.left_ = -1.0f;
    camera_config.right_ = 1.0f;
    camera_config.top_ = 1.0f;
    camera_config.down_ = -1.0f;
    PerspectiveCamera camera;
    camera.init(camera_config);
    view_and_projection_transforms[i] = mmdot<float, 4>(
        camera.projection_transform_, camera.world_to_view_transform_);
    {
      auto &difuse_texture = *(difuse_images[i].get());
      ibl_irradiance_convolution_precompution(renderer, camera, difuse_texture,
                                              model_pool, room_instance,
                                              cube_map_texture);
    }
  }
  auto diffuse_cube_texture = six_image_to_cub_mapping_texture(difuse_images);
  return diffuse_cube_texture;
}
std::shared_ptr<Texture> compute_prefilter_texture(
    Renderer *renderer, const ModelPool &model_pool, int width, int height,
    std::shared_ptr<Texture> &cube_map_texture, float roughness,
    int mip_level_for_debug, bool is_dump) {
  auto room_instance =
      Instance{"uv_inner_cube", Vec3{0.0f, 0.0f, 0.0f},
               Vec3{50.0f, 50.0f, 50.0f}, eye<float, 3>(), false};
  std::vector<Square4> view_and_projection_transforms;
  view_and_projection_transforms.resize(6);
  std::vector<Vec3> camera_forward_directions{
      //+x -x // left right
      Vec3{1.0f, 0.0f, 0.0f},
      Vec3{-1.0f, 0.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, -1.0f, 0.0f},
      //+z -z // front back
      Vec3{0.0f, 0.0f, 1.0f},
      Vec3{0.0f, 0.0f, -1.0f},
  };
  std::vector<Vec3> camera_up_directions{
      //+x -x // left right
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
      //+y -y // top bottom
      Vec3{0.0f, 0.0f, -1.0f},
      Vec3{0.0f, 0.0f, 1.0f},
      //+z -z // front back
      Vec3{0.0f, 1.0f, 0.0f},
      Vec3{0.0f, 1.0f, 0.0f},
  };
  std::vector<std::string> depth_image_names{
      "left",  "right",  //
      "top",   "bottom", //
      "front", "back",   //
  };
  std::vector<std::unique_ptr<SSAADepthImage>> prefiter_images;
  // Make it small, else dramatically ssssslllllloooowwww;
  for (int i = 0; i < 6; i++) {
    prefiter_images.push_back(
        std::make_unique<SSAADepthImage>(width, height, 1));
  }
  for (int i = 0; i < 6; i++) {
    std::cout << "roughness: " << roughness << " prfilter: " << i << "\n";
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{0.0f, 0.0f, 0.0f};
    camera_config.forward_ = camera_forward_directions[i];
    camera_config.up_ = camera_up_directions[i];
    camera_config.far_ = -100.0f;
    camera_config.near_ = -1.0f;
    camera_config.left_ = -1.0f;
    camera_config.right_ = 1.0f;
    camera_config.top_ = 1.0f;
    camera_config.down_ = -1.0f;
    PerspectiveCamera camera;
    camera.init(camera_config);
    view_and_projection_transforms[i] = mmdot<float, 4>(
        camera.projection_transform_, camera.world_to_view_transform_);
    {
      auto &difuse_texture = *(prefiter_images[i].get());
      ibl_prefilter_precompution(renderer, camera, difuse_texture, model_pool,
                                 room_instance, cube_map_texture, roughness);
      if (i == 0 && is_dump) {
        difuse_texture.dump(std::to_string(mip_level_for_debug) + "_" +
                            std::to_string(i) + "_prefilter");
      }
    }
  }
  auto prefilter_cube_texture =
      six_image_to_cub_mapping_texture(prefiter_images);
  return prefilter_cube_texture;
}
int main() {
  try {
    Vec3 world_centor = Vec3{0.0f, 0.0f, 0.0f};
    size_t height = 1000;
    size_t width = 1000;
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{5.0f, 0.0f, 3.0f};
    // camera_config.position_ = Vec3{0.0f, 3.0f, 5.0f};
    camera_config.forward_ = world_centor - camera_config.position_;
    camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
    camera_config.far_ = -100.0f;
    camera_config.near_ = -3.0f;
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
    auto cube_map_texture = convert_hdr_equidistant_cylindrical_to_cube_mapping(
        renderer.get(), model_pool, width, height);
    auto diffuse_cube_texture = compute_irradiance_convolution_texture(
        renderer.get(), model_pool, cube_map_texture);
    std::shared_ptr<Texture> prfilter_mip_cube_mapping_texture{nullptr};
    {
      int max_mip_levels = 5;
      std::vector<std::shared_ptr<CubeMapTexture>> prefilter_cube_map_textures;
      for (unsigned int mip = 0; mip < max_mip_levels; ++mip) {
        std::cout << "prefilter mip level: " << mip << "\n";
        int mip_width = 128 * std::pow(0.5, mip);
        int mip_height = 128 * std::pow(0.5, mip);
        float roughness = (float)mip / (float)(max_mip_levels - 1);
        bool is_dump{true};
        auto prefilter_mip_cube_mapping_texture = compute_prefilter_texture(
            renderer.get(), model_pool, mip_width, mip_height, cube_map_texture,
            roughness, mip, is_dump);
        prefilter_cube_map_textures.push_back(
            std::dynamic_pointer_cast<CubeMapTexture>(
                prefilter_mip_cube_mapping_texture));
      }
      prfilter_mip_cube_mapping_texture = std::dynamic_pointer_cast<Texture>(
          std::make_shared<LevelsTexture>(prefilter_cube_map_textures));
    }
    std::shared_ptr<Texture> prdf_LUT_texture{nullptr};
    {
      std::cout << "commputing brdf_LUT\n";
      size_t brdf_LUT_width = 512;
      size_t brdf_LUT_height = 512;
      SSAADepthImage prdf_LUT{brdf_LUT_width, brdf_LUT_height, 1};
      ibl_specular_brdf_compution(renderer.get(), prdf_LUT, model_pool);
      prdf_LUT.dump("brdf_LUT");
      prdf_LUT_texture =
          move_ssaa_depth_image_to_innner_texture_image(&prdf_LUT);
    }
    std::vector<std::shared_ptr<Texture>> albedo_textures;
    std::vector<std::shared_ptr<Texture>> ao_textures;
    std::vector<std::shared_ptr<Texture>> metallic_textures;
    std::vector<std::shared_ptr<Texture>> normal_textures;
    std::vector<std::shared_ptr<Texture>> roughness_textures;
    std::vector<std::string> material_type{"gold", "wall", "plastic", "grass",
                                           "rusted_iron"};

    {
      std::cout << "loading texture\n";
      int scale{1};
      for (auto &type : material_type) {
        auto albedo_texture = make_scale_image_texture(
            "../../resources/textures/" + type + "/albedo.png", scale);
        albedo_textures.push_back(albedo_texture);
        auto ao_texture = make_scale_image_texture(
            "../../resources/textures/" + type + "/ao.png", scale);
        ao_textures.push_back(ao_texture);
        auto metallic_texture = make_scale_image_texture(
            "../../resources/textures/" + type + "/metallic.png", scale);
        metallic_textures.push_back(metallic_texture);
        auto normal_texture = make_scale_image_texture(
            "../../resources/textures/" + type + "/normal.png", scale);
        normal_textures.push_back(normal_texture);
        auto roughness_texture = make_scale_image_texture(
            "../../resources/textures/" + type + "/roughness.png", scale);
        roughness_textures.push_back(roughness_texture);
      }
    }
    {
      std::cout << "deferred geometry pass running ...\n";
      std::vector<std::shared_ptr<PBRLight>> lights;
      lights.push_back(make_ibl_diffuse_spcular_pbr_ambient_light());
      lights.push_back(
          make_pbr_point_light(world_centor + Vec3{0.0f, 0.0f, 5.0f},
                               Color{300.0f, 300.0f, 300.0f}));
      // int scale{1};
      // auto albedo_texture = make_scale_image_texture(
      //     "../../resources/textures/rusted_iron/albedo.png", scale);
      // auto ao_texture = make_scale_image_texture(
      //     "../../resources/textures/rusted_iron/ao.png", scale);
      // auto metallic_texture = make_scale_image_texture(
      //     "../../resources/textures/rusted_iron/metallic.png", scale);
      // auto normal_texture = make_scale_image_texture(
      //     "../../resources/textures/rusted_iron/normal.png", scale);
      // auto roughness_texture = make_scale_image_texture(
      //     "../../resources/textures/rusted_iron/roughness.png", scale);
      SSAAGBufferDepthImage g_buffer_image{width, height, SSAA};
      size_t material_type_num = material_type.size();
      int rows{5};
      int columns{5};
      float spacing = 1.1;
      float ao = 1.0f;
      Color albedo = Color{0.5f, 0.0f, 0.0f};
      for (int row = 0; row < rows; ++row) {
        float metallic = (float)row / (float)rows;
        for (int col = 0; col < columns; ++col) {
          float roughness = clamp((float)col / (float)columns, 0.05f, 1.0f);
          auto translate = Vec3{(col - (columns / 2)) * spacing,
                                (row - (rows / 2)) * spacing, 0.0f};
          auto uv_sphere_instance =
              Instance{"uv_sphere", translate, Vec3{1.0f, 1.0f, 1.0f},
                       eye<float, 3>(), false};
          auto material_index = (row * col) % material_type_num;
          pbr_texture_uv_deferred_geometry_rendering_homoposition_uv_norm(
              renderer.get(), camera, g_buffer_image, model_pool,
              uv_sphere_instance, albedo_textures[material_index],
              metallic_textures[material_index],
              roughness_textures[material_index], ao_textures[material_index],
              normal_textures[material_index]);
        }
      }
      CHECK(prfilter_mip_cube_mapping_texture != nullptr)
      CHECK(prdf_LUT_texture != nullptr)
      std::cout << "deferred light pass running ...\n";
      ibl_diffuse_specular_pbr_uv_deferred_lights_rendering(
          renderer.get(), camera, image, model_pool, lights, g_buffer_image,
          diffuse_cube_texture, prfilter_mip_cube_mapping_texture,
          prdf_LUT_texture);
      std::cout << "skybox rendering running ...\n";
      image.depth_buffer_ = std::move(g_buffer_image.depth_buffer_);
      bool is_enable_depth_test = true;
      sky_box_render_homo_view_position(renderer.get(), camera, image,
                                        model_pool, cube_map_texture,
                                        is_enable_depth_test);
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
}