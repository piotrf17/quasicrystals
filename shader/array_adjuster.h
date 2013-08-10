// ArrayAdjuster provides a gui element for adjusting parameters in an array,
// where the relative values of parameters are more important than their
// absolute values.  Notably, this gui does not use any text.
//
// Note that this assumes a screen scaled to be [0, 1] for x and y.  Use
// glScale and glTranslate before calling Draw().

#ifndef QUASICRYSTAL_ARRAY_ADJUSTER_H
#define QUASICRYSTAL_ARRAY_ADJUSTER_H

namespace quasicrystal {

class ArrayAdjuster {
 public:
  // Create a new ArrayAdjuster:
  //   v - Array that will be modified by this adjuster
  //   size - number of elements in v
  //   interval - interval for drawing guidelines
  ArrayAdjuster(float* v, int size, float interval);

  // Move our selection left or right.
  void SelectLeft();
  void SelectRight();

  // Modify the selected array parameter by the specified amount.
  void Adjust(float amount);

  // Draw the adjuster GUI using OpenGL.  By default, the gui is drawn using
  // black bars, with the selected value drawn in bright green.
  void Draw();

  // Later calls to draw won't show anything.  However, any call to
  // Select{Left,Right} or Adjust will unhide the array adjuster.
  void Hide();
  
 private:
  float* v_;
  int size_;
  float interval_;
  int selection_;
  bool hidden_;
};

}  // namespace quasicrystal

#endif
