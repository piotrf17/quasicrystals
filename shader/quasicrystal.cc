#include <stdio.h>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include "textfile.h"

DEFINE_int32(width, 400, "Width of output image.");
DEFINE_int32(height, 400, "Height of output image.");

GLuint f,p;
float a = 0;
static int width, height;

void changeSize(int w, int h) {
  width = w;
  height = h;
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void renderScene(void) {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  /*  gluLookAt(0.0,0.0,5.0, 
            0.0,0.0,-1.0,
            0.0f,1.0f,0.0f);*/
  /*
  glBegin(GL_QUADS);
  glVertex3f(-1.0, -1.0, 0.0);
  glVertex3f(1.0, -1.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(-1.0, 1.0, 0.0);
  glEnd();
  */

  glBegin(GL_QUADS);
  glVertex2i(0, 0);
  glVertex2i(width, 0);
  glVertex2i(width, height);
  glVertex2i(0, height);
  glEnd();

  
  a+=0.1;

  glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {
  if (key == 27) 
    exit(0);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{
  //
  // Returns 1 if an OpenGL error occurred, 0 otherwise.
  //
  GLenum glErr;
  int    retCode = 0;

  glErr = glGetError();
  while (glErr != GL_NO_ERROR)
  {
    printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
    retCode = 1;
    glErr = glGetError();
  }
  return retCode;
}


void printShaderInfoLog(GLuint obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;

  glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

  if (infologLength > 0)
  {
    infoLog = (char *)malloc(infologLength);
    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
    printf("%s\n",infoLog);
    free(infoLog);
  }
}

void printProgramInfoLog(GLuint obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;

  glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

  if (infologLength > 0)
  {
    infoLog = (char *)malloc(infologLength);
    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
    printf("%s\n",infoLog);
    free(infoLog);
  }
}



void setShaders() {
  char *fs = NULL;

  f = glCreateShader(GL_FRAGMENT_SHADER);

  fs = textFileRead("qc.frag");
  const char * ff = fs;
  glShaderSource(f, 1, &ff,NULL);

  free(fs);

  glCompileShader(f);

  printShaderInfoLog(f);

  p = glCreateProgram();
  glAttachShader(p,f);

  glLinkProgram(p);
  printProgramInfoLog(p);

  glUseProgram(p);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(FLAGS_width, FLAGS_height);
  glutCreateWindow("quasicrystal"); 

  glutDisplayFunc(renderScene);
  glutIdleFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutKeyboardFunc(processNormalKeys);

  glClearColor(0.0, 0.0, 0.0, 1.0);

  glewInit();
  if (glewIsSupported("GL_VERSION_2_0"))
    printf("Ready for OpenGL 2.0\n");
  else {
    printf("OpenGL 2.0 not supported\n");
    exit(1);
  }
  
  setShaders();

  glutMainLoop();

  return 0;
}

