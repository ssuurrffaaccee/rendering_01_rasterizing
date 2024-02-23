#include <vector>

#include "base_indicator.hpp"
#include "cube.hpp"
#include "depth_image.hpp"
// #include "render.hpp"
#include "blinn_phong_light_model.hpp"
#include "camera.hpp"
#include "clipping_geometry_shader.hpp"
#include "light.hpp"
#include "renderer.hpp"
#include "sphere.hpp"
World build_world() {
  World world{};
  {
    SphereGenerator1 sphere_generator{7};
    Model sphere = sphere_generator.get();
    for (auto &p : sphere.points_) {
      sphere.colors_.push_back(abs(p));
    }
    // Cone cone_generator{3};
    // Model cone = cone_generator.get();
    // for (auto &p : cone.points_) {
    //   cone.colors_.push_back(abs(p));
    // }
    Model test_model;
    test_model.points_ =
        std::vector<Vec3>{Vec3{0.0f, 1.0f, -0.01f}, Vec3{-1.0f, 0.0f, -0.01f},
                          Vec3{1.0f, 0.0f, -0.01f}};
    test_model.colors_ =
        std::vector<Color>{Color{1.0f, 0.0f, 0.0f}, Color{0.0f, 1.0f, 0.0f},
                           Color{0.0f, 0.0f, 1.0f}};
    test_model.triangles_.push_back(std::tuple<int,int,int>(0,1,2));
    auto color_shpere = sphere;
    color_shpere.colors_.clear();
    for (auto &p : color_shpere.points_) {
      color_shpere.colors_.push_back(Color{1.0f, 0.0f, 0.0f});
    }
    world.models_["sphere"] = std::move(sphere);
    Model cube = get_cube();
    world.models_["cube"] = std::move(cube);
    Model base_indicator = get_base_indicator();
    world.models_["base"] = std::move(base_indicator);
    // world.models_["cone"] = std::move(cone);
    world.models_["color_sphere"] = std::move(color_shpere);
    world.models_["test"] = test_model;
  }
  world.instances_.push_back(Instance{"base", Vec3{0.0f, 0.0f, 0.0f},
                                      Vec3{0.5f, 0.5f, 0.5f}, eye<float, 3>(),
                                      false});
  // world.instances_.push_back(Instance{"sphere", Vec3{0.0f, 0.0f, 0.0f},
  //                                     Vec3{4.0f, 4.0f, 2.0f}, eye<float,
  //                                     3>(), false});
  world.instances_.push_back(Instance{"color_sphere", Vec3{0.0f, 0.0f, 0.0f},
                                      Vec3{2.00f, 2.00f, 2.00f},
                                      eye<float, 3>(), false});
  // world.instances_.push_back(Instance{"test", Vec3{0.0f, 0.0f, 0.0f},
  //                                     Vec3{1.0f, 1.0f, 1.0f}, eye<float,
  //                                     3>(), false});
  // world.instances_.push_back(Instance{"test", Vec3{0.0f, 0.0f, -1.0f},
  //                                     Vec3{-1.0f, -1.0f, 1.0f}, eye<float,
  //                                     3>(), false});
  return world;
}
std::vector<VertexStageData> model_to_vertices(const Model &model) {
  std::vector<VertexStageData> vertices;
  for (int i = 0; i < model.points_.size(); i++) {
    VertexStageData data;
    // homo_position
    data.insert(data.end(), model.points_[i].begin(), model.points_[i].end());
    data.push_back(1.0f);
    // color
    data.insert(data.end(), model.colors_[i].begin(), model.colors_[i].end());
    // norm
    auto norm = normalize(model.points_[i]);
    data.insert(data.end(), norm.begin(), norm.end());
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
    camera_config.position_ = Vec3{0.f, 0.0f, 2.1f};
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
    World world = build_world();
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    // vertex shader
    std::shared_ptr<VertexShader> vertex_shader = std::shared_ptr<VertexShader>{
        (VertexShader *)new BlinnPhongVertexShader{}};
    std::shared_ptr<FragmentShader> fragment_shader =
        std::shared_ptr<FragmentShader>{
            (FragmentShader *)new BlinnPhongFragmentShader{}};
    std::shared_ptr<GeometryShader> gemoetry_shader =
        std::shared_ptr<GeometryShader>{
            (GeometryShader *)new ClippingTriangleGeometryShader_7{}};
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(make_directional_light(Vec3{-0.5f, -0.5f, -0.5f},
                                            Color{1.0f, 1.0f, 1.0f}));
    lights.push_back(make_ambient_light(Color{0.12f, 0.12f, 0.12f}));
    // lights.push_back(make_point_light(world_centor + Vec3{0.0f, 2.0f, 0.0f},
    //                                   Color{1.0f, 1.0f, 1.0f}));
    renderer->geometry_shader_ = gemoetry_shader.get();
    renderer->fragment_shader_ = fragment_shader.get();
    renderer->vertex_shader_ = vertex_shader.get();
    renderer->fragment_shader_->set_global_data(0, (void *)&lights);
    renderer->fragment_shader_->set_global_data(1, (void *)&camera.position_);
    renderer->vertex_shader_->set_global_data(
        0, (void *)&camera.world_to_view_transform_);
    renderer->vertex_shader_->set_global_data(
        1, (void *)&camera.projection_transform_);
    renderer->geometry_shader_->set_global_data(
        1, (void *)&camera.clipping_planes_);
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