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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#include <gflags/gflags.h>
#include <GL/glew.h>
#include <GL/glut.h>

DEFINE_int32(width, 600, "Width of output image.");
DEFINE_int32(height, 600, "Height of output image.");
DEFINE_int32(num_waves, 7, "Initial number of waves.");
DEFINE_double(spatial_freq, 1.0 / 5.0, "Initial spatial frequency of waves.");
DEFINE_string(shader_source, "qc.frag",
              "Path to fragment shader source code");
DEFINE_string(phases,
              "1.0, 0.9, 0.8, 0.7, 0.6,"
              "0.5, 0.4, 0.3, 0.2, 0.1,"
              "0.1, 0.2, 0.3, 0.4, 0.5",
              "Comma seperated list of phases, maximum of 15.");
DEFINE_double(time_granularity, 0.01,
              "Parameter that controls granularity in modifying the speed.");

void LoadShaders();

// Keep in sync with constant in qc.frag
const int kMaxNumWaves = 15;

// Global variables, woooo!
static GLuint p;                // handle to shader program
static GLuint phases_texture;   // handle to texture containing phases
static int width, height;       // screen params
static float t, dt;             // passage of time
static bool is_paused = false;
static int num_waves;          
static float spatial_freq;
static float mix, mixv;

void InitializeParameters() {
  num_waves = std::max(std::min(FLAGS_num_waves, kMaxNumWaves), 1);
  spatial_freq = FLAGS_spatial_freq;

  t = 0.0;
  dt = 5 * FLAGS_time_granularity;
  mix = 0.0; mixv = 0.0;

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
  glGenTextures(1, &phases_texture);
  glBindTexture(GL_TEXTURE_1D, phases_texture);
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
}

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
  if (!is_paused) {
    t += dt;

    mix += mixv;
    if (mix < 0) {
      mix = 0.0;
      mixv = 0.0;
    }
    if (mix > 1) {
      mix = 0.0;
      mixv = 0.0;
      ++num_waves;
    }
  }
  
  // Pass in all parameters to the shader.
  GLint t_loc = glGetUniformLocation(p, "t");
  glUniform1f(t_loc, t);
  GLint num_waves_loc = glGetUniformLocation(p, "num_waves");
  glUniform1i(num_waves_loc, num_waves);
  GLint spatial_freq_loc = glGetUniformLocation(p, "spatial_freq");
  glUniform1f(spatial_freq_loc, spatial_freq);
  GLint mix_loc = glGetUniformLocation(p, "mix");
  glUniform1f(mix_loc, mix);
  GLint phases_loc = glGetUniformLocation(p, "phases");
  glUniform1i(phases_loc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, phases_texture);

  // Reset drawing state.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // Draw our canvas, this should invoke the shaders.
  glBegin(GL_QUADS);
  glVertex2i(0, 0);
  glVertex2i(width, 0);
  glVertex2i(width, height);
  glVertex2i(0, height);
  glEnd();

  glutSwapBuffers();
}

void HandleKeypress(unsigned char key, int x, int y) {
  switch (key) {
    case '[':
      if (mixv > 0.0) {
        mixv = -0.01;
      } else if (mixv < 0.0) {
        mix = 0.0; mixv = 0.0;
      } else if (num_waves > 1) {
        --num_waves;
        mix = 1.0; mixv = -0.01;
      }
      break;
    case ']':
      if (mixv < 0.0) {
        mixv = 0.01;
      } else if (mixv > 0.0) {
        mix = 0.0; mixv = 0.0;
        ++num_waves;
      } else if (num_waves < kMaxNumWaves - 1) {
        mix = 0.0; mixv = 0.01;
      }
      break;
    case '.':
      dt += FLAGS_time_granularity;
      break;
    case ',':
      dt -= FLAGS_time_granularity;
      break;
    case ' ':
      is_paused = !is_paused;
      break;
    case '-':
      spatial_freq *= 1.1;
      break;
    case '=':
      spatial_freq /= 1.1;
      break;
    case 'r':
      LoadShaders();
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
  GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

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

  // Setup any other random OpenGL drawing stuff.
  glClearColor(0.0, 0.0, 0.0, 1.0);

  // Intialize glew and compile our shader.
  glewInit();
  if (glewIsSupported("GL_VERSION_2_0"))
    printf("Ready for OpenGL 2.0\n");
  else {
    printf("OpenGL 2.0 not supported\n");
    exit(1);
  }
  LoadShaders();

  // Setup initial parameters to pass into the shader.
  InitializeParameters();

  glutMainLoop();

  return 0;
}

