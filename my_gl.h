#include "geometry.h"
#include "tgaimage.h"

void lookAt(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int widht, const int height);

typedef vec4 Triangle[3];
struct IShader {
  virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};
void rasterize(const Triangle &clip, const IShader &shader,
               TGAImage &framebuffer);
