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

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <GL/glew.h>
#include <GL/glut.h>

DEFINE_int32(width, 400, "Width of output image.");
DEFINE_int32(height, 400, "Height of output image.");
DEFINE_int32(num_waves, 7, "Initial number of waves.");
DEFINE_double(freq, 1.0 / 5.0, "Initial spatial frequency of waves.");
DEFINE_string(shader_source, "qc.frag",
              "Path to fragment shader source code");

// Keep in sync with constant in qc.frag
const int kMaxNumWaves = 15;

// Global variables, woooo!
static GLuint f,p;
static float t = 0.0;
static int width, height;
static int num_waves;
static float freq;
static float dt = 0.1;
static bool isPaused = false;

void HandleResize(int w, int h) {
  width = w;
  height = h;
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Update resolution in shader.
  GLint resolution_loc = glGetUniformLocation(p, "resolution");
  glUniform2f(resolution_loc, static_cast<float>(w), static_cast<float>(h));
}

void RenderScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  // Our canvas is just a square that fills the screen.
  glBegin(GL_QUADS);
  glVertex2i(0, 0);
  glVertex2i(width, 0);
  glVertex2i(width, height);
  glVertex2i(0, height);
  glEnd();

  // Set uniform variables.
  if (!isPaused) {
    t += dt;
  }
  GLint t_loc = glGetUniformLocation(p, "t");
  glUniform1f(t_loc, t);
  GLint num_waves_loc = glGetUniformLocation(p, "num_waves");
  glUniform1i(num_waves_loc, num_waves);
  GLint freq_loc = glGetUniformLocation(p, "freq");
  glUniform1f(freq_loc, freq);

  glutSwapBuffers();
}

void HandleKeypress(unsigned char key, int x, int y) {
  switch (key) {
    case '[':
      num_waves = std::max(1, num_waves - 1);
      break;
    case ']':
      num_waves = std::min(kMaxNumWaves, num_waves + 1);
      break;
    case '.':
      dt += 0.1;
      break;
    case ',':
      dt -= 0.1;
      break;
    case ' ':
      isPaused = !isPaused;
      break;
    case '-':
      freq *= 1.1;
      break;
    case '=':
      freq /= 1.1;
      break;
    case 27:  // escape
      exit(0);
  }
}

void PrintShaderInfoLog(GLuint obj) {
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;

  glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 0) {
    infoLog = new char[infologLength];
    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
    std::cout << infoLog << std::endl;
    delete[] infoLog;
  }
}

void PrintProgramInfoLog(GLuint obj) {
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;

  glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

  if (infologLength > 0) {
    infoLog = new char[infologLength];
    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
    std::cout << infoLog << std::endl;
    delete[] infoLog;
  }
}

void LoadShaders() {
  f = glCreateShader(GL_FRAGMENT_SHADER);

  // Load the shader source code.
  std::ifstream infile(FLAGS_shader_source);
  std::string source_str((std::istreambuf_iterator<char>(infile)),
                         std::istreambuf_iterator<char>());
  const char* source = source_str.c_str();
  glShaderSource(f, 1, &source, NULL);

  // Compile and print any errors.
  glCompileShader(f);
  PrintShaderInfoLog(f);

  p = glCreateProgram();
  glAttachShader(p,f);

  glLinkProgram(p);
  PrintProgramInfoLog(p);

  glUseProgram(p);
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // Initialize some flag based parameters.
  num_waves = FLAGS_num_waves;
  freq = static_cast<float>(FLAGS_freq);

  // Create a new window with GLUT.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(FLAGS_width, FLAGS_height);
  glutCreateWindow("quasicrystal"); 

  // Register Callbacks.
  glutDisplayFunc(RenderScene);
  glutIdleFunc(RenderScene);
  glutReshapeFunc(HandleResize);
  glutKeyboardFunc(HandleKeypress);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  
  glewInit();
  if (glewIsSupported("GL_VERSION_2_0"))
    printf("Ready for OpenGL 2.0\n");
  else {
    printf("OpenGL 2.0 not supported\n");
    exit(1);
  }
  
  LoadShaders();

  glutMainLoop();

  return 0;
}

