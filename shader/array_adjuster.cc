#include "array_adjuster.h"

#include <algorithm>

#include <GL/gl.h>

namespace quasicrystal {

ArrayAdjuster::ArrayAdjuster(float* v, int size, float interval)
    : v_(v),
      size_(size),
      interval_(interval),
      selection_(0),
      hidden_(false) {
}

void ArrayAdjuster::SelectLeft() {
  hidden_ = false;
  selection_ = std::max(0, selection_ - 1);
}

void ArrayAdjuster::SelectRight() {
  hidden_ = false;
  selection_ = std::min(size_ - 1, selection_ + 1);
}

void ArrayAdjuster::Adjust(float amount) {
  hidden_ = false;
  v_[selection_] += amount;
}

void ArrayAdjuster::Draw() {
  if (hidden_) {
    return;
  }
  float max = 0.0001;
  for (int i = 0; i < size_; ++i) {
    max = std::max(std::abs(v_[i]), max);
  }
  max = ceil(max / interval_) * interval_;
  float yscale = 0.4 / max;
  float width = std::min(0.2, 0.8 / size_);
  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_LINES);
  for (float y = interval_; y <= max; y += interval_) {
    glVertex2f(0.05, y * yscale + 0.5);
    glVertex2f(0.95, y * yscale + 0.5);
  }
  for (float y = -interval_; y >= -max; y -= interval_) {
    glVertex2f(0.05, y * yscale + 0.5);
    glVertex2f(0.95, y * yscale + 0.5);
  }
  glEnd();
  glBegin(GL_QUADS);
  glVertex2f(0.05, 0.497);
  glVertex2f(0.95, 0.497);
  glVertex2f(0.95, 0.503);
  glVertex2f(0.05, 0.503);
  for (int i = 0; i < size_; ++i) {
    if (i == selection_) {
      glColor3f(0.0, 1.0, 0.0);
    } else {
      glColor3f(0.0, 0.0, 0.0);
    }
    float x = 0.1 + i * width + 0.1 * width;
    float y = v_[i] * yscale + 0.5;
    glVertex2f(x + 0.1 * width, y - 0.01);
    glVertex2f(x + 0.9 * width, y - 0.01);
    glVertex2f(x + 0.9 * width, y + 0.01);
    glVertex2f(x + 0.1 * width, y + 0.01);
  }
  glEnd();
}

void ArrayAdjuster::Hide() {
  hidden_ = true;
}

}  // namespace quasicrystal
