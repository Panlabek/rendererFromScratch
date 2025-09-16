#ifndef MODEL_H
#define MODEL_H
#include "geometry.h"
#include "tgaimage.h"
#include <array>
#include <string>
#include <vector>
struct Model {
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<vec2> uv_map;
  std::vector<std::array<int, 9>> faces_v_idx;
  TGAImage normalmap = {};
  TGAImage diffusemap = {};
  bool load_obj(const std::string &path);
  vec4 normal(const vec2 &uv) const;
  TGAColor diffuse(const vec2 &uv) const;
};
#endif
