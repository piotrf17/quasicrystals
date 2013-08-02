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
#include <SFML/Window.hpp>

#include "shader_util.h"
#include "window.h"

DEFINE_int32(width, 600, "Width of output image.");
DEFINE_int32(height, 600, "Height of output image.");
DEFINE_string(shader_source, "qc.frag",
              "Path to fragment shader source code");
DEFINE_int32(num_waves, 7, "Initial number of waves.");
DEFINE_double(spatial_freq, 1.0 / 5.0, "Initial spatial frequency of waves.");
DEFINE_string(phases,
              "1.0, 0.9, 0.8, 0.7, 0.6,"
              "0.5, 0.4, 0.3, 0.2, 0.1,"
              "0.1, 0.2, 0.3, 0.4, 0.5",
              "Comma seperated list of phases, maximum of 15.");
DEFINE_double(time_granularity, 0.01,
              "Parameter that controls granularity in modifying the speed.");

// Keep in sync with constant in qc.frag
const int kMaxNumWaves = 15;

class QCWindow : public Window2d {
 public:
  QCWindow(int width, int height) :
      Window2d(width, height, "quasicrystal") {
  }

 protected:
  virtual void Init() {
    Window2d::Init();

    // Initialize glew and compile our shaders.
    glewInit();
    if (glewIsSupported("GL_VERSION_2_0")) {
      std::cout << "Ready for OpenGL 2.0" << std::endl;
    } else {
      std::cout << "ERROR: OpenGL 2.0 not supported" << std::endl;
      Close();
    }
    std::string debug;
    if (!ShaderUtil::BuildShaderFromFile(
            FLAGS_shader_source, GL_FRAGMENT_SHADER, &shader_, &debug)) {
      std::cout << "ERROR: failed to load shader from " << FLAGS_shader_source
                << std::endl << debug;
      Close();
    }

    // Initialize parameters.
    num_waves_ = std::max(std::min(FLAGS_num_waves, kMaxNumWaves), 1);
    spatial_freq_ = FLAGS_spatial_freq;

    t_ = 0.0;
    dt_ = 5 * FLAGS_time_granularity;
    mix_ = 0.0; mixv_ = 0.0;

    // Split comma seperated string into floats.  I should just use boost...
    GLfloat* phases = new GLfloat[kMaxNumWaves];
    std::stringstream ss(FLAGS_phases);
    std::string phase_str;
    int i = 0;
    while (std::getline(ss, phase_str, ',') && i < kMaxNumWaves) {
      phases[i] = atof(phase_str.c_str());
      ++i;
    }
  
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
                 phases);
  
    // I think this should setup sampling to be close enough to selecting
    // values from the array.  I haven't thoroughly tested.
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  
    delete[] phases;

    is_paused_ = false;
  }
  
  virtual void Draw() {
    if (!is_paused_) {
      t_ += dt_;

      mix_ += mixv_;
      if (mix_ < 0) {
        mix_ = 0.0;
        mixv_ = 0.0;
      }
      if (mix_ > 1) {
        mix_ = 0.0;
        mixv_ = 0.0;
        ++num_waves_;
      }
    }
  
    // Pass in all parameters to the shader.
    GLint t_loc = glGetUniformLocation(shader_, "t");
    glUniform1f(t_loc, t_);
    GLint num_waves_loc = glGetUniformLocation(shader_, "num_waves");
    glUniform1i(num_waves_loc, num_waves_);
    GLint spatial_freq_loc = glGetUniformLocation(shader_, "spatial_freq");
    glUniform1f(spatial_freq_loc, spatial_freq_);
    GLint mix_loc = glGetUniformLocation(shader_, "mix");
    glUniform1f(mix_loc, mix_);
    GLint phases_loc = glGetUniformLocation(shader_, "phases");
    glUniform1i(phases_loc, 0);
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

  virtual void Keypress(int key) {
    switch (key) {
      case 10:  // [
        if (mixv_ > 0.0) {
          mixv_ = -0.01;
        } else if (mixv_ < 0.0) {
          mix_ = 0.0; mixv_ = 0.0;
        } else if (num_waves_ > 1) {
          --num_waves_;
          mix_ = 1.0; mixv_ = -0.01;
        }
        break;
      case 11:  // ]
        if (mixv_ < 0.0) {
          mixv_ = 0.01;
        } else if (mixv_ > 0.0) {
          mix_ = 0.0; mixv_ = 0.0;
          ++num_waves_;
        } else if (num_waves_ < kMaxNumWaves - 1) {
          mix_ = 0.0; mixv_ = 0.01;
        }
        break;
      case 14:  // '.'
        dt_ += FLAGS_time_granularity;
        break;
      case 13:  // ','
        dt_ -= FLAGS_time_granularity;
        break;
      case 21:  // ' '
        is_paused_ = !is_paused_;
        break;
      case 20:  // '-'
        spatial_freq_ *= 1.1;
        break;
      case 19:  // '='
        spatial_freq_ /= 1.1;
        break;
      default:
        break;
    }
  }

 private:
  GLuint shader_;
  GLuint phases_texture_;
  float t_, dt_;
  bool is_paused_;
  int num_waves_;
  float spatial_freq_;
  float mix_, mixv_;
};

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  QCWindow window(FLAGS_width, FLAGS_height);
  window.Run();
  
  return EXIT_SUCCESS;
}
