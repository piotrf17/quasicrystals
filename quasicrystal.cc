// Based on code by Matthew Peddie:
// https://github.com/peddie/quasicrystals/
// which is in turn based on code from Keegan McAllister:
// http://mainisusuallyafunction.blogspot.com/2011/10/quasicrystals-as-sums-of-waves-in-plane.html

#include <cmath>
#include <iostream>
#include <vector>

#include <gflags/gflags.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "window.h"

DEFINE_int32(width, 640, "Width of output image.");
DEFINE_int32(height, 480, "Height of output image.");
DEFINE_int32(num_waves, 7, "Number of waves to use.");
DEFINE_double(freq, 1.0 / 5.0, "Frequency of waves.");
DEFINE_bool(view_mode, true,
            "Set to true to run visualization, set to false to "
            "run benchmark");

static void ComputeWave(float* img, int step) {
  const float freq = static_cast<float>(FLAGS_freq);

  // Initializes phases and angles.
  std::vector<float> coses(FLAGS_num_waves);
  std::vector<float> sines(FLAGS_num_waves);
  for (int i = 0; i < FLAGS_num_waves; ++i) {
    float angle = i * M_PI / FLAGS_num_waves;
    coses[i] = cos(angle);
    sines[i] = sin(angle);
  }

  for (int y = 0; y < FLAGS_height; ++y) {
    for (int x = 0; x < FLAGS_width; ++x) {
      const int idx = FLAGS_width * y + x;
      float p = 0;
      for (int w = 0; w < FLAGS_num_waves; w += 1) {
        const float cx = coses[w] * x;
        const float sy = sines[w] * y;
        const float phase = step * 0.05 * (w + 1);
        p += (cos(freq * (cx + sy) + phase) + 1) / 2;
      }
      p = (cos(M_PI * p) + 1) / 2;
      //      const uint8_t tmp = static_cast<uint8_t>(
      //          255 * std::min(1.0f, std::max(p, 0.0f)));
      img[idx] = p;
    }
  }

}

class WaveWindow : public util::Window {
 public:
  WaveWindow()
      : util::Window("Quasicrystal", FLAGS_width, FLAGS_height),
        pixels_(new float [FLAGS_width * FLAGS_height]),
        step_(0) {
  }
  virtual ~WaveWindow() {
    delete pixels_;
  }

 protected:
  virtual void HandleClose() {
    exit(0);
  }

  virtual void HandleKey(unsigned int state, unsigned int keycode) {
  }

  virtual void HandleDraw() {
    ++step_;
    ComputeWave(pixels_, step_);
    
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glRasterPos2i(0, 0);
    glDrawPixels(FLAGS_width,
                 FLAGS_height,
                 GL_LUMINANCE, GL_FLOAT,
                 pixels_);
  }

 private:
  float* pixels_;
  int step_;
};

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (XInitThreads() == 0) {
    std::cout << "Failed to initialize thread support in xlib." << std::endl;
    return 1;
  }

  WaveWindow window;

  getchar();
}