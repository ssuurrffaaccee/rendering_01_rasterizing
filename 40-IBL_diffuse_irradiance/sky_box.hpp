#pragma once
#include "clipping.hpp"
#include "light.hpp"
#include "shader.hpp"
#include "texture.hpp"

VertexStageData transform_in_view(Square4 &view_rotate_transform,
                                  const VertexStageData &vertex) {
  Vec4 homo_world_vector{vertex[0], vertex[1], vertex[2], 0.0f}; // no translate
  auto new_vertex = vertex;
  auto p_in_view = mvdot<float, 4>(view_rotate_transform, homo_world_vector);
  new_vertex[0] = p_in_view[0];
  new_vertex[1] = p_in_view[1];
  new_vertex[2] = p_in_view[2];
  // interpolating on -1to1 cube surface
  new_vertex.insert(new_vertex.end(), homo_world_vector.begin(),
                    homo_world_vector.end() - 1);
  return new_vertex;
}
class SkyBoxShader : public Shader {
public:
  std::vector<VertexStageData>
  process_triangle(const VertexStageData &v_0, const VertexStageData &v_1,
                   const VertexStageData &v_2) override {
    std::vector<Plane> &clipping_planes_in_view_space_ =
        *(std::vector<Plane> *)get_global_data(0);
    // all vertex as vector not point, so no need translate, just rotate it.
    Square4 &view_transform = *(Square4 *)get_global_data(1);
    auto veiw_v_0 = transform_in_view(view_transform, v_0);
    auto view_v_1 = transform_in_view(view_transform, v_1);
    auto view_v_2 = transform_in_view(view_transform, v_2);
    // std::cout<<"\n";
    // for(auto& plane : clipping_planes_in_view_space_){
    //   std::cout<<to_string(plane.outside_direction_)<<"
    //   "<<to_string(plane.sample_point_)<<"\n";
    // }
    // std::cout<<"\n";
    return clipping(veiw_v_0, view_v_1, view_v_2,
                    clipping_planes_in_view_space_);
  }
  VertexStageData process_vertex(const VertexStageData &vertex) override {
    Square4 &projection_transform = *(Square4 *)get_global_data(2);
    Vec4 p_in_view{vertex[0], vertex[1], vertex[2], vertex[3]};
    auto new_vertex = vertex;
    auto p_in_projection = mvdot<float, 4>(projection_transform, p_in_view);
    new_vertex[0] = p_in_projection[0];
    new_vertex[1] = p_in_projection[1];
    new_vertex[2] = p_in_projection[2];
    new_vertex[3] = p_in_projection[3];
    // perspective correct
    {
      float w = new_vertex[3];
      for (int i = 0; i < new_vertex.size(); i++) {
        new_vertex[i] /= w;
      }
      new_vertex[3] = 1.0f / w;
    }
    return new_vertex;
  }
  std::tuple<Vec<int, 2>, Color, float>
  process_fragment(const FragmentStageData &frag) override {
    float depth = frag.second[2];
    Texture &cube_map_texture = *(Texture *)get_global_data(3);
    Vec3 view_dirction = Color{frag.second[4], frag.second[5], frag.second[6]};
    Color color = cube_map_texture.sample(view_dirction[0], view_dirction[1],
                                          view_dirction[2]);
    // Color color = Color{1.0f,0.0f,0.0f};
    //depth = std::numeric_limits<float>::lowest();
    depth = -1.0f;
    return std::make_tuple(frag.first, color, depth);
  }
};

REGISTER_SHADER(Shader, sky_box, SkyBoxShader)