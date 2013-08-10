// Quasicrystals, shader version.
// Requires OpenGL 2.0 or better.
//
// Controls:
//   [  and  ]   decrease or increase number of waves
//   -  and  =   decrease or increase spatial frequency (zoom)
//   ,  and  .   decrease or increase speed
//   spacebar    pause
// 
// Idea based on code by Matthew Peddie:
// https://github.com/peddie/quasicrystals/
// which is in turn based on code from Keegan McAllister:
// http://mainisusuallyafunction.blogspot.com/2011/10/quasicrystals-as-sums-of-waves-in-plane.html
//
// Used of GLSL and shaders based on the excellent tutorial on Lighthouse3D:
// http://www.lighthouse3d.com/tutorials/glsl-tutorial/

#include <iostream>
#include <sstream>
#include <string>

#include <gflags/gflags.h>
#include <GL/glew.h>

#include "shader_util.h"
#include "window.h"

DEFINE_int32(width, 600, "Width of output image.");
DEFINE_int32(height, 600, "Height of output image.");
DEFINE_string(shader_source, "qc.frag",
              "Path to fragment shader source code");
DEFINE_int32(num_waves, 7, "Initial number of waves, max of 15.");
DEFINE_double(wavenumber, 0.2, "Initial spatial frequency of waves.");
DEFINE_string(angular_frequencies,
              "1.0, 0.9, 0.8, 0.7, 0.6,"
              "0.5, 0.4, 0.3, 0.2, 0.1,"
              "0.1, 0.2, 0.3, 0.4, 0.5",
              "Comma seperated list of initial wave angular frequencies.");
DEFINE_double(time_granularity, 0.01,
              "Parameter that controls granularity in modifying the speed.");

// Keep in sync with constant in qc.frag
const int kMaxNumWaves = 15;

namespace {

// A sufficient set of parameters to describe any snapshot of the quasicrystals.
struct QCParams {
  QCParams()
      : t(0.0),
        num_waves(1),
        mix(0.0),
        angular_frequencies({1.0}),
        wavenumber(0.2) {}
  // Current time in the wave propagation.
  float t;
  // Number of waves.
  int num_waves;
  // Wave number mixing parameter, 0 = num_waves, 1 = num_waves + 1
  float mix;
  // Angular frequencies of each wave, individually specified.
  float angular_frequencies[kMaxNumWaves];
  // Spatial frequency of all of the waves.
  // TODO(piotrf): specify per wave wavenumbers.
  float wavenumber;
};

QCParams InitQCParamsFromFlags() {
  QCParams params;
  params.num_waves = std::max(std::min(FLAGS_num_waves, kMaxNumWaves), 1);
  params.wavenumber = FLAGS_wavenumber;
  // Split comma seperated string into floats.  I should just use boost...
  std::stringstream ss(FLAGS_angular_frequencies);
  std::string phase_str;
  int i = 0;
  while (std::getline(ss, phase_str, ',') && i < kMaxNumWaves) {
    params.angular_frequencies[i] = atof(phase_str.c_str());
    ++i;
  }
  return params;
}

class QCWindow : public graphics::Window2d {
 public:
  QCWindow(int width, int height, const QCParams& params)
      : Window2d(width, height, "quasicrystal"),
        params_(params) {
  }

 protected:
  virtual bool Init() {
    if (!Window2d::Init()) {
      return false;
    }

    // Initialize glew and compile our shaders.
    glewInit();
    if (glewIsSupported("GL_VERSION_2_0")) {
      std::cout << "Ready for OpenGL 2.0" << std::endl;
    } else {
      std::cout << "ERROR: OpenGL 2.0 not supported" << std::endl;
      return false;
    }
    std::string debug;
    if (!ShaderUtil::BuildShaderFromFile(
            FLAGS_shader_source, GL_FRAGMENT_SHADER, &shader_, &debug)) {
      std::cout << "ERROR: failed to load shader from " << FLAGS_shader_source
                << std::endl << debug;
      return false;
    }

    // Initialize any simulation variables outside of params.
    is_paused_ = false;
    dt_ = 5 * FLAGS_time_granularity;
    mixv_ = 0.0;

    // Pass phases into the shader via a 1-d texture.  
    glEnable(GL_TEXTURE_1D);
    glGenTextures(1, &phases_texture_);
    glBindTexture(GL_TEXTURE_1D, phases_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D,
                 0,             // LOD level, 0 = base image
                 GL_R32F,       // packing type
                 kMaxNumWaves,  // size
                 0,             // border
                 GL_RED,        // pixel data format
                 GL_FLOAT,      // pixel data type
                 params_.angular_frequencies);
  
    // I think this should setup sampling to be close enough to selecting
    // values from the array.  I haven't thoroughly tested.
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    return true;
  }
  
  virtual void Draw() {
    if (!is_paused_) {
      params_.t += dt_;

      params_.mix += mixv_;
      if (params_.mix < 0) {
        params_.mix = 0.0;
        mixv_ = 0.0;
      }
      if (params_.mix > 1) {
        params_.mix = 0.0;
        mixv_ = 0.0;
        ++params_.num_waves;
      }
    }
  
    // Pass in all parameters to the shader.
    GLint t_loc = glGetUniformLocation(shader_, "t");
    glUniform1f(t_loc, params_.t);
    GLint num_waves_loc = glGetUniformLocation(shader_, "num_waves");
    glUniform1i(num_waves_loc, params_.num_waves);
    GLint spatial_freq_loc = glGetUniformLocation(shader_, "spatial_freq");
    glUniform1f(spatial_freq_loc, params_.wavenumber);
    GLint mix_loc = glGetUniformLocation(shader_, "mix");
    glUniform1f(mix_loc, params_.mix);
    GLint phases_loc = glGetUniformLocation(shader_, "phases");
    glUniform1i(phases_loc, 0);
    // Update resolution in shader.
    GLint resolution_loc = glGetUniformLocation(shader_, "resolution");
    glUniform2f(resolution_loc, static_cast<float>(width()),
                static_cast<float>(height()));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, phases_texture_);

    // Reset drawing state.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Draw our canvas, this should invoke the shaders.
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(width(), 0);
    glVertex2i(width(), height());
    glVertex2i(0, height());
    glEnd();
  }

  virtual void Keypress(unsigned int key) {
    switch (key) {
      case XK_bracketleft:
        if (mixv_ > 0.0) {
          mixv_ = -0.01;
        } else if (mixv_ < 0.0) {
          params_.mix = 0.0; mixv_ = 0.0;
        } else if (params_.num_waves > 1) {
          --params_.num_waves;
          params_.mix = 1.0; mixv_ = -0.01;
        }
        break;
      case XK_bracketright:
        if (mixv_ < 0.0) {
          mixv_ = 0.01;
        } else if (mixv_ > 0.0) {
          params_.mix = 0.0; mixv_ = 0.0;
          ++params_.num_waves;
        } else if (params_.num_waves < kMaxNumWaves - 1) {
          params_.mix = 0.0; mixv_ = 0.01;
        }
        break;
      case XK_period:
        dt_ += FLAGS_time_granularity;
        break;
      case XK_comma:
        dt_ -= FLAGS_time_granularity;
        break;
      case XK_space:
        is_paused_ = !is_paused_;
        break;
      case XK_minus:
        params_.wavenumber *= 1.1;
        break;
      case XK_equal:
        params_.wavenumber /= 1.1;
        break;
      case XK_Escape:
        Close();
        break;
      default:
        break;
    }
  }

 private:
  GLuint shader_;
  GLuint phases_texture_;
  bool is_paused_;
  float dt_, mixv_;
  QCParams params_;
};

}  // namespace

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  QCWindow window(FLAGS_width, FLAGS_height, InitQCParamsFromFlags());
  window.Run();
  
  return EXIT_SUCCESS;
}
