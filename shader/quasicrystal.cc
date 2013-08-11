// Quasicrystals, shader version.
// Requires OpenGL 2.0 or better.
//
// Controls:
//   [  and  ]   decrease or increase number of waves
//   -  and  =   decrease or increase spatial frequency (zoom)
//   ,  and  .   decrease or increase speed
//   spacebar    pause
//   a, d        move angular frequency selector left or right
//   w, s        increase / decrease selected angular frequency
//   j, l        move wavenumber selector left or right
//   i, k        increase / decrease selected wavenumber
//   q           close angular frequency or wavenumber selector
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

#include "array_adjuster.h"
#include "shader_util.h"
#include "window.h"

DEFINE_int32(width, 600, "Width of output image.");
DEFINE_int32(height, 600, "Height of output image.");
DEFINE_string(shader_source, "qc.frag",
              "Path to fragment shader source code");
DEFINE_int32(num_waves, 7, "Initial number of waves, max of 15.");
DEFINE_string(wavenumbers,
              "0.2, 0.2, 0.2, 0.2, 0.2,"
              "0.2, 0.2, 0.2, 0.2, 0.2,"
              "0.2, 0.2, 0.2, 0.2, 0.2",
              "Comma seperated list of initial per wave wavenumbers.");
DEFINE_string(angular_frequencies,
              "1.0, 0.9, 0.8, 0.7, 0.6,"
              "0.5, 0.4, 0.3, 0.2, 0.1,"
              "0.1, 0.2, 0.3, 0.4, 0.5",
              "Comma seperated list of initial wave angular frequencies.");
DEFINE_double(time_granularity, 0.01,
              "Parameter that controls granularity in modifying the speed.");

// Keep in sync with constant in qc.frag
const int kMaxNumWaves = 15;

using graphics::ShaderUtil;

namespace quasicrystal {

// A sufficient set of parameters to describe any snapshot of the quasicrystals.
struct QCParams {
  QCParams()
      : t(0.0),
        num_waves(1),
        mix(0.0),
        angular_frequencies({1.0}),
        wavenumbers({0.2}) {}
  // Current time in the wave propagation.
  float t;
  // Number of waves.
  int num_waves;
  // Wave number mixing parameter, 0 = num_waves, 1 = num_waves + 1
  float mix;
  // Angular frequencies of each wave, individually specified.
  float angular_frequencies[kMaxNumWaves];
  // Spatial frequency of each wave, individually specified.
  float wavenumbers[kMaxNumWaves];
};

void SplitCommaSeparatedFloats(const std::string& str, float* v, int size) {
  std::stringstream ss(str);
  std::string temp;
  int i = 0;
  while (std::getline(ss, temp, ',') && i < size) {
    v[i] = atof(temp.c_str());
    ++i;
  }
}

QCParams InitQCParamsFromFlags() {
  QCParams params;
  params.num_waves = std::max(std::min(FLAGS_num_waves, kMaxNumWaves), 1);
  SplitCommaSeparatedFloats(
      FLAGS_angular_frequencies, params.angular_frequencies, kMaxNumWaves);
  SplitCommaSeparatedFloats(
      FLAGS_wavenumbers, params.wavenumbers, kMaxNumWaves);
  return params;
}

// A class that bridges a set of quasicrystal parameters and a shader.
class QCShaderParams {
 public:
  QCShaderParams(const QCParams* params) : params_(params), shader_(0) {
  }

  // Initialize our connection to the shader with the given handle.
  void Init(GLuint shader) {
    shader_ = shader;
    // The vector of angular frequencies is passed as a texture.
    glGenTextures(1, &angular_frequencies_texture_);
    glBindTexture(GL_TEXTURE_1D, angular_frequencies_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, kMaxNumWaves,
                 0, GL_RED, GL_FLOAT, params_->angular_frequencies);
    // I think this should setup sampling to be close enough to selecting
    // values from the array.  I haven't thoroughly tested.
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // The vector of wavenumbers is also passed as a texture.
    glGenTextures(1, &wavenumbers_texture_);
    glBindTexture(GL_TEXTURE_1D, wavenumbers_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, kMaxNumWaves,
                 0, GL_RED, GL_FLOAT, params_->wavenumbers);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  }

  // Send our parameters down to the shader.
  // Make sure the names match up with the variable names in the shader.
  void UpdateShaderParams() {
    GLint t_loc = glGetUniformLocation(shader_, "t");
    glUniform1f(t_loc, params_->t);
    GLint num_waves_loc = glGetUniformLocation(shader_, "num_waves");
    glUniform1i(num_waves_loc, params_->num_waves);
    GLint mix_loc = glGetUniformLocation(shader_, "mix");
    glUniform1f(mix_loc, params_->mix);
    GLint angular_frequencies_loc =
        glGetUniformLocation(shader_, "angular_frequencies");
    glUniform1i(angular_frequencies_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, angular_frequencies_texture_);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, kMaxNumWaves, GL_RED, GL_FLOAT,
                    params_->angular_frequencies);
    GLint wavenumbers_loc =
        glGetUniformLocation(shader_, "wavenumbers");
    glUniform1i(wavenumbers_loc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, wavenumbers_texture_);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, kMaxNumWaves, GL_RED, GL_FLOAT,
    params_->wavenumbers);
  }
  
  void set_shader(GLuint shader) {shader_ = shader;}
  
 private:
  const QCParams* params_;
  GLuint shader_;
  GLuint angular_frequencies_texture_;
  GLuint wavenumbers_texture_;
};

class QCWindow : public graphics::Window2d {
 public:
  QCWindow(int width, int height, const QCParams& params)
      : Window2d(width, height, "quasicrystal"),
        params_(params),
        shader_params_(&params_) {
  }

 protected:
  virtual bool Init() {
    if (!Window2d::Init()) {
      return false;
    }

    glEnable(GL_TEXTURE_1D);
    
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

    // Setup the bridge between our params and the shader params.
    shader_params_.Init(shader_);

    // Initialize any simulation variables outside of params.
    is_paused_ = false;
    dt_ = 5 * FLAGS_time_granularity;
    mixv_ = 0.0;
    
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
  
    // Pass in all QC parameters to the shader.
    shader_params_.UpdateShaderParams();

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

    if (af_adjuster.get() != nullptr || wn_adjuster.get() != nullptr) {
      // TODO(piotrf): this is really a terrible way to disable the shader,
      // figure out a cleaner way.
      glUseProgram(0);
      // Disable textures so that we can just draw with colors.
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_1D);
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_1D);
      glPushMatrix();
      glScalef(width(), height(), 1.0);
      if (af_adjuster.get() != nullptr) {
        af_adjuster->Draw();
      } else if (wn_adjuster.get() != nullptr) {
        wn_adjuster->Draw();
      }
      glPopMatrix();
      // Re-enable textures so that we can just draw with colors.
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_1D);
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_1D);
      glUseProgram(shader_);
    }
  }

  virtual void Resize(int width, int height) {
    Window2d::Resize(width, height);
    // Update resolution in shader.
    GLint resolution_loc = glGetUniformLocation(shader_, "resolution");
    glUniform2f(resolution_loc, static_cast<float>(width),
                static_cast<float>(height));
  }

  virtual void Keypress(unsigned int key) {
    switch (key) {
      case XK_bracketleft:
        if (af_adjuster.get() != nullptr) {
          af_adjuster.reset(nullptr);
        }
        if (wn_adjuster.get() != nullptr) {
          wn_adjuster.reset(nullptr);
        }
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
        if (af_adjuster.get() != nullptr) {
          af_adjuster.reset(nullptr);
        }
        if (wn_adjuster.get() != nullptr) {
          wn_adjuster.reset(nullptr);
        }
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
        for (int i = 0; i < kMaxNumWaves; ++i) {
          params_.wavenumbers[i] *= 1.1;
        }
        break;
      case XK_equal:
        for (int i = 0; i < kMaxNumWaves; ++i) {
          params_.wavenumbers[i] /= 1.1;
        }
        break;
      case XK_Escape:
        Close();
        break;
      case XK_a: case XK_A:
        if (af_adjuster.get() == nullptr) {
          if (mixv_ == 0.0 &&
              (wn_adjuster.get() == nullptr || wn_adjuster->hidden())) {
            wn_adjuster.reset();
            af_adjuster.reset(new ArrayAdjuster(
                params_.angular_frequencies, params_.num_waves, 0.5));
          }
        } else {
          af_adjuster->SelectLeft();
        }
        break;
      // Angular frequency adjuster controls.
      case XK_d: case XK_D:
        if (af_adjuster.get() == nullptr) {
          if (mixv_ == 0.0 &&
              (wn_adjuster.get() == nullptr || wn_adjuster->hidden())) {
            wn_adjuster.reset();
            af_adjuster.reset(new ArrayAdjuster(
                params_.angular_frequencies, params_.num_waves, 0.5));
          }
        } else {
          af_adjuster->SelectRight();
        }
        break;
      case XK_w: case XK_W:
        if (af_adjuster.get() != nullptr) {
          af_adjuster->Adjust(0.1);
        }
        break;
      case XK_s: case XK_S:
        if (af_adjuster.get() != nullptr) {
          af_adjuster->Adjust(-0.1);
        }
        break;
      // Wavenumber adjuster controls.
      case XK_j: case XK_J:
        if (wn_adjuster.get() == nullptr) {
          if (mixv_ == 0.0 &&
              (af_adjuster.get() == nullptr || af_adjuster->hidden())) {
            af_adjuster.reset();
            wn_adjuster.reset(new ArrayAdjuster(
                params_.wavenumbers, params_.num_waves, 0.5));
          }
        } else {
          wn_adjuster->SelectLeft();
        }
        break;
      case XK_l: case XK_L:
        if (wn_adjuster.get() == nullptr) {
          if (mixv_ == 0.0 &&
              (af_adjuster.get() == nullptr || af_adjuster->hidden())) {
            af_adjuster.reset();
            wn_adjuster.reset(new ArrayAdjuster(
                params_.wavenumbers, params_.num_waves, 0.5));
          }
        } else {
          wn_adjuster->SelectRight();
        }
        break;
      case XK_i: case XK_I:
        if (wn_adjuster.get() != nullptr) {
          wn_adjuster->Adjust(0.1);
        }
        break;
      case XK_k: case XK_K:
        if (wn_adjuster.get() != nullptr) {
          wn_adjuster->Adjust(-0.1);
        }
        break;
      case XK_q: case XK_Q:
        if (af_adjuster.get() != nullptr) {
          af_adjuster->Hide();
        } else if (wn_adjuster.get() != nullptr) {
          wn_adjuster->Hide();
        }
        break;
      default:
        break;
    }
  }

 private:
  GLuint shader_;                  // Shader program handle.
  QCParams params_;                // Mathematical params for the quasicrystal.
  QCShaderParams shader_params_;   // Link between our params and shader.
  bool is_paused_;                 // Is the simulation paused or not?
  float dt_, mixv_;                // Time step, mixing velocity.

  // GUI element for adjusting angular frequencies.
  std::unique_ptr<ArrayAdjuster> af_adjuster;
  // GUI element for adjutsing wavenumbers.
  std::unique_ptr<ArrayAdjuster> wn_adjuster;
};

}  // namespace quasicrystal

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  quasicrystal::QCWindow window(
      FLAGS_width, FLAGS_height, quasicrystal::InitQCParamsFromFlags());
  window.Run();
  
  return EXIT_SUCCESS;
}
