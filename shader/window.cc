#include "window.h"

#include <SFML/Window.hpp>

Window::Window(int width, int height, const std::string& title)
    : sf_window_(new sf::Window(sf::VideoMode(width, height),
                                title)) {      
}

Window::~Window() {
}

void Window::Run() {
  // Initialize any 
  Init();
  Resize(sf_window_->GetWidth(), sf_window_->GetHeight());
  while (sf_window_->IsOpened()) {
    sf::Event event;
    while (sf_window_->GetEvent(event)) {
      switch (event.Type) {
        case sf::Event::Closed:
          sf_window_->Close();
          break;
        case sf::Event::Resized:
          Resize(event.Size.Width, event.Size.Height);
          break;
        case sf::Event::KeyPressed:
          if (event.Key.Code == sf::Key::Escape) {
            sf_window_->Close();
          } else {
            Keypress(event.Key.Code);
          }
          break;
        default:
          break;
      }
    }
    sf_window_->SetActive();
    Draw();
    sf_window_->Display();
  }
}

void Window::Init() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

void Window::Close() {
  sf_window_->Close();
}

int Window::width() const {
  return sf_window_->GetWidth();
}

int Window::height() const {
  return sf_window_->GetHeight();
}

Window2d::Window2d(int width, int height, const std::string& title)
    : Window(width, height, title) {
}

void Window2d::Resize(int width, int height) {
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, 0, height, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
