#include "geometry.h"
#include "model.h"
#include "my_gl.h"
#include "tgaimage.h"
#include <limits>

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;
struct PhongShader : IShader {
  const Model &model;
  vec4 l;
  vec4 tri[3];
  vec4 normals[3];
  vec2 uv_normals[3];
  PhongShader(const vec3 light, const Model &m) : model(m) {
    // l = normalized(ModelView * vec4{light.x, light.y, light.z, 0}).xyz();
    l = normalized(ModelView * vec4{light.x, light.y, light.z, 0});
  }
  virtual vec4 vertex(const int face, const int vert) {
    vec3 v = model.positions[model.faces_v_idx[face][vert]];
    vec3 n = model.normals[model.faces_v_idx[face][vert + 3]];
    normals[vert] = (ModelView.invert_transpose() * vec4{n.x, n.y, n.z, 0});
    uv_normals[vert] = model.uv_map[model.faces_v_idx[face][vert + 6]];
    vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1};
    tri[vert] = gl_Position;
    return Perspective * gl_Position;
  }
  virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
    // TGAColor gl_FragColor = {255, 255, 255, 255};
    // mat<2, 4> E = {tri[1] - tri[0], tri[2] - tri[0]};
    mat<2, 4> E = {tri[1] - tri[0], tri[2] - tri[0]};
    mat<2, 2> U = {uv_normals[1] - uv_normals[0],
                   uv_normals[2] - uv_normals[0]};
    mat<2, 4> T = U.invert() * E;
    mat<4, 4> D = {normalized(T[0]),
                   normalized(T[1]),
                   normalized(normals[0] * bar[0] + normals[1] * bar[1] +
                              normals[2] * bar[2]),
                   {0, 0, 0, 1}};
    vec2 uv = uv_normals[0] * bar[0] + uv_normals[1] * bar[1] +
              uv_normals[2] * bar[2];
    TGAColor gl_FragColor = model.diffuse(uv);
    vec4 n = normalized(D.transpose() * model.normal(uv));
    vec4 r = normalized(n * (n * l) * 2 - l);
    double ambient = .3;
    double diff = std::max(0., n * l);
    double specular = std::pow(std::max(r.z, 0.), 35);
    for (int channel : {0, 1, 2}) {
      gl_FragColor[channel] *=
          std::min(1., ambient + .6 * diff + .9 * specular);
    }
    return {false, gl_FragColor};
  }
};

int main(int argc, char **argv) {
  constexpr int width = 800;
  constexpr int height = 800;
  constexpr vec3 light{1, 1, 1};
  constexpr vec3 eye{-1, 0, 2};   // camera position
  constexpr vec3 center{0, 0, 0}; // camera direction
  constexpr vec3 up{0, 1, 0};     // camera up vector
  TGAImage framebuffer(width, height, TGAImage::RGB);
  std::vector<double> zbuffer(width * height,
                              -std::numeric_limits<double>::max());
  lookAt(eye, center, up);
  init_perspective(norm(eye - center));
  init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
  init_zbuffer(width, height);
  for (int m = 1; m < argc; m++) {
    Model model;
    model.load_obj(argv[m]);
    PhongShader shader(light, model);
    for (int i = 0; i < model.faces_v_idx.size(); i++) {
      Triangle clip = {shader.vertex(i, 0), shader.vertex(i, 1),
                       shader.vertex(i, 2)};
      rasterize(clip, shader, framebuffer);
    }
  }
  framebuffer.write_tga_file("framebuffer.tga");
  return 0;
}
