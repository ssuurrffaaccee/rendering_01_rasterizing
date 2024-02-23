#include <vector>

#include "base_indicator.hpp"
#include "cube.hpp"
#include "depth_image.hpp"
// #include "render.hpp"
#include "camera.hpp"
#include "check_texture.hpp"
#include "clipping_geometry_shader.hpp"
#include "renderer.hpp"
#include "simple_shader.hpp"
#include "sphere.hpp"
World build_world() {
  World world{};
  {
    Model test_model;
    auto p0 = Vec3{-1.0f, 0.0f, -0.01f};
    auto p1 = Vec3{1.0f, 0.0f, -0.01f};
    auto p2 = Vec3{-1.0f, 0.0f, -10.0f};
    auto p3 = Vec3{1.0f, 0.0f, -10.0f};

    test_model.points_ = std::vector<Vec3>{p2, p0, p1, p1, p3, p2};
    test_model.triangles_.push_back(std::tuple<int, int, int>(0, 1, 2));
    test_model.triangles_.push_back(std::tuple<int, int, int>(3, 4, 5));
    test_model.uvs_ =
        std::vector<Vec2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
                          {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    world.models_["test"] = test_model;
  }
  world.instances_.push_back(Instance{"test", Vec3{0.0f, 0.0f, 0.0f},
                                      Vec3{1.0f, 1.0f, 1.0f}, eye<float, 3>(),
                                      false});
  return world;
}
std::vector<VertexStageData> model_to_vertices(const Model &model) {
  std::vector<VertexStageData> vertices;
  for (int i = 0; i < model.points_.size(); i++) {
    VertexStageData data;
    // position
    data.insert(data.end(), model.points_[i].begin(), model.points_[i].end());
    data.push_back(1.0f);
    // uv
    data.insert(data.end(), model.uvs_[i].begin(), model.uvs_[i].end());
    vertices.push_back(std::move(data));
  }
  return vertices;
}
int main() {
  try {
    Vec3 world_centor = Vec3{0.0f, 0.0f, 0.0f};
    size_t height = 1000;
    size_t width = 1000;
    PerspectiveCameraConfig camera_config;
    camera_config.position_ = Vec3{0.0f, 1.0f, 1.0f};
    camera_config.forward_ = world_centor - camera_config.position_;
    camera_config.up_ = Vec3{0.0f, 1.0f, 0.0f};
    camera_config.far_ = -10.0f;
    camera_config.near_ = -1.0f;
    camera_config.left_ = -1.0f;
    camera_config.right_ = 1.0f;
    camera_config.top_ = 1.0f;
    camera_config.down_ = -1.0f;
    PerspectiveCamera camera;
    camera.init(camera_config);
    DepthImage image{width, height};
    World world = build_world();
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    // vertex shader
    std::shared_ptr<VertexShader> vertex_shader =
        std::shared_ptr<VertexShader>{(VertexShader *)new SimpleVertexShader{}};
    std::shared_ptr<FragmentShader> fragment_shader =
        std::shared_ptr<FragmentShader>{
            (FragmentShader *)new SimpleFragmentShader{}};
    std::shared_ptr<GeometryShader> gemoetry_shader =
        std::shared_ptr<GeometryShader>{
            (GeometryShader *)new ClippingTriangleGeometryShader{}};
    renderer->geometry_shader_ = gemoetry_shader.get();
    renderer->fragment_shader_ = fragment_shader.get();
    renderer->vertex_shader_ = vertex_shader.get();
    renderer->vertex_shader_->set_global_data(
        0, (void *)&camera.world_to_view_transform_);
    renderer->vertex_shader_->set_global_data(
        1, (void *)&camera.projection_transform_);
    renderer->geometry_shader_->set_global_data(
        1, (void *)&camera.clipping_planes_);
    std::shared_ptr<CheckerTexture> check_texture =
        std::make_shared<CheckerTexture>();
    renderer->fragment_shader_->set_global_data(0, (void *)check_texture.get());
    renderer->color_buffer_ = image.data_->data();
    renderer->depth_buffer_ = image.depth_buffer_->data();
    renderer->frame_width_ = width;
    renderer->frame_height_ = height;
    for (auto &instance : world.instances_) {
      auto iter = world.models_.find(instance.name_);
      CHECK(iter != world.models_.end())
      auto model_transform = instance.to_model_transform();
      renderer->geometry_shader_->set_global_data(0, (void *)&model_transform);
      auto vertices = model_to_vertices(iter->second);
      renderer->is_draw_framework_ = instance.is_frame_;
      renderer->render(vertices, iter->second.triangles_);
    }
    image.dump("xxxx");
  } catch (MyExceptoin &e) {
    std::cout << e.what() << "\n";
  }
}