#include "model.h"
#include "tgaimage.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static inline int first_number_before_slash(const std::string &token) {
  size_t slash = token.find('/');
  std::string head =
      (slash == std::string::npos) ? token : token.substr(0, slash);
  return std::stoi(head);
}
static inline int number_after_last_slash(const std::string &token) {
  size_t slash = token.rfind('/');
  std::string tail =
      (slash == std::string::npos) ? token : token.substr(slash + 1);
  return std::stoi(tail);
}
static inline int number_after_first_slash(const std::string &s) {
  const std::size_t first = s.find('/');
  if (first == std::string::npos || first + 1 >= s.size())
    throw std::runtime_error("no slash or nothing after slash");

  const std::size_t second = s.find('/', first + 1);
  const std::string mid =
      s.substr(first + 1, (second == std::string::npos ? std::string::npos
                                                       : second - (first + 1)));
  if (mid.empty())
    throw std::runtime_error("empty field between slashes");
  return std::stoi(mid);
}

bool Model::load_obj(const std::string &path) {
  positions.clear();
  faces_v_idx.clear();

  std::ifstream in(path);
  if (!in)
    return false;

  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream iss(line);
    std::string tag;
    iss >> tag;
    if (tag == "v") {
      vec3 v{};
      iss >> v.x >> v.y >> v.z;
      positions.push_back(v);
    } else if (tag == "f") {
      std::vector<int> vidx_raw;
      std::vector<int> normals_raw;
      std::vector<int> uv_raw;
      std::string tok;
      while (iss >> tok) {
        vidx_raw.push_back(first_number_before_slash(tok));
        uv_raw.push_back(number_after_first_slash(tok));
        normals_raw.push_back(number_after_last_slash(tok));
      }

      auto to_zero_based = [&](int idx1) -> int {
        if (idx1 > 0)
          return idx1 - 1;
        return static_cast<int>(positions.size()) + idx1;
      };
      faces_v_idx.push_back(
          {to_zero_based(vidx_raw[0]), to_zero_based(vidx_raw[1]),
           to_zero_based(vidx_raw[2]), to_zero_based(normals_raw[0]),
           to_zero_based(normals_raw[1]), to_zero_based(normals_raw[2]),
           to_zero_based(uv_raw[0]), to_zero_based(uv_raw[1]),
           to_zero_based(uv_raw[2])});
    } else if (tag == "vn") {
      vec3 v{};
      iss >> v.x >> v.y >> v.z;
      normals.push_back(v);
    } else if (tag == "vt") {
      vec2 v{};
      iss >> v.x >> v.y;
      uv_map.push_back(v);
    }
  }
  auto load_texture = [&path](const std::string suffix, TGAImage &img) {
    size_t dot = path.find_last_of(".");
    if (dot == std::string::npos)
      return;
    std::string texFile = path.substr(0, dot) + suffix;
    std::cerr << "texture file " << texFile << " loading "
              << (img.read_tga_file(texFile.c_str()) ? "ok" : "failed")
              << std::endl;
  };
  load_texture("_nm_tangent.tga", normalmap);
  load_texture("_diffuse.tga", diffusemap);
  normalmap.flip_vertically();
  diffusemap.flip_vertically();
  return true;
}

vec4 Model::normal(const vec2 &uv) const {
  TGAColor c =
      normalmap.get(uv[0] * normalmap.width(), uv[1] * normalmap.height());
  return normalized(vec4{(double)c[2], (double)c[1], (double)c[0], 0} * 2. /
                        255 -
                    vec4{1, 1, 1, 0});
}
TGAColor Model::diffuse(const vec2 &uv) const {
  TGAColor c =
      diffusemap.get(uv[0] * diffusemap.width(), uv[1] * diffusemap.height());
  return c;
}
