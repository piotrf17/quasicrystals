// Some basic window classes that simplify creating an application with an
// OpenGL window.  These classes setup reasonable defaults, and simply leave
// a few functions to be overridden:
//   Keypress(): called whenever a key is _pressed_ (not released)
//   Draw(): called after handling all events
//
// If you want to do anything interesting or complicated, just use a library
// like SFML directly.  Assumptions made:
//   * 32 bit color space, default window settings
//   * press Escape, or close the window to quit (no notification!)
//   * only keyboard input, and only key pressed handled
// 
// The classes provided in this file are:
// Window - A bare bones class that handles an event loop.
// Window2d - Sets up an orthographic projection with pixel coordinates
//   in the rectangle (0, 0) - (width, height)


#ifndef QUASICRYSTAL_WINDOW_H
#define QUASICRYSTAL_WINDOW_H

#include <memory>
#include <string>

namespace sf {
  class Window;
}  // namespace sf

class Window {
 public:
  Window(int width, int height, const std::string& title);
  virtual ~Window();

  // Start running the application.  Starts off by calling Init(), Resize(),
  // and then enters an event handling loop.  Each loop calls Draw(),
  // and may call Keypress() if there are keypress events to handle.
  void Run();

 protected:
  // For initializing any application state or OpenGL stuff.
  virtual void Init();

  // Handle resizing the window to a new width and height.
  virtual void Resize(int width, int height) = 0;

  // Handle the initial press of a key.
  virtual void Keypress(int key) = 0;

  // Handle redrawing the frame.
  virtual void Draw() = 0;

  // Request that the window is closed, also ends the event loop in Run(). 
  void Close();

  int width() const;
  int height() const;
  
 private:
  std::unique_ptr<sf::Window> sf_window_;
};

class Window2d : public Window {
 public:
  Window2d(int width, int height, const std::string& title);

 protected:
  virtual void Resize(int width, int height);
};

#endif
