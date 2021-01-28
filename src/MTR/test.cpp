#include <iostream>
#include <memory>

#include "MTR/math_defs.h"
#include "MTR/softras/render.hpp"

struct Test_A2V {
  vec3f position;
  vec3f color;
};

struct Test_V2F {
  vec3f color;
};

class Test_Shader : public IShader<Test_A2V, Test_V2F> {
  virtual vec4f vertex_shader(Test_A2V &a2v, Test_V2F &v2f) {
    v2f.color = a2v.color;
    // v2f.view_pos = (a2v.model * a2v.view * position).head<3>();

    vec4f position;
    position << a2v.position, 1.0f;
    return project * model * view * position;
  }
  virtual vec4f fragment_shader(Test_V2F &v2f) {
    vec4f result;
    result << v2f.color, 1.0f;
    return result;
  }
  virtual Test_V2F interpolate_attr(Test_V2F *v2f, vec3f bc_weight) {
    Test_V2F result;
    result.color = bc_interpolate_attr(v2f[0].color, v2f[1].color, v2f[2].color,
                                       bc_weight);
    return result;
  }
};

int main() {
  int test_width = 500, test_height = 500;
  auto shader = std::make_shared<Test_Shader>();
  auto raster = std::make_shared<SoftRaster<decltype(shader->arr2vert_cls),
                                            decltype(shader->vert2frag_cls)>>(
      test_width, test_height);
  raster->test();

  raster->set_shader(shader);
  raster->set_project(
      perspective(60.0f, test_width / (float)test_height, 0.1f, 50.0f));

  std::vector<Test_A2V> vertex_data;
  Test_A2V a, b, c;
  a.position << 0.5f, 0.5f, 1.5f;
  a.color << 1.0f, 0.0f, 0.0f;
  b.position << -0.5f, 0.5f, 2.0f;
  b.color << 0.0f, 1.0f, 0.0f;
  c.position << 0.5f, -0.5f, 2.5f;
  c.color << 0.0f, 0.0f, 1.0f;
  vertex_data.insert(vertex_data.end(), {a, b, c});

  auto buffer_id = raster->load_array(vertex_data);
  raster->draw_arrays(buffer_id);

  auto &frame_buf = raster->frame_buffer();

  FILE *fp = fopen("test.ppm", "wb");
  (void)fprintf(fp, "P6\n%d %d\n255\n", test_width, test_height);
  for (auto i = 0; i < test_height * test_width; ++i) {
    static unsigned char color[3];
    color[0] = (unsigned char)(255 * clamp(0, 1, frame_buf[i].x()));
    color[1] = (unsigned char)(255 * clamp(0, 1, frame_buf[i].y()));
    color[2] = (unsigned char)(255 * clamp(0, 1, frame_buf[i].z()));
    if (frame_buf[i].x() > 0.5f) {
      printf("make color (%f, %f, %f)->(%d, %d, %d).\n", frame_buf[i].x(),
             frame_buf[i].y(), frame_buf[i].z(), (int)color[0], (int)color[1],
             (int)color[2]);
    }
    fwrite(color, 1, 3, fp);
  }
  fclose(fp);

  printf("hello from MTR\n");
  return 0;
}