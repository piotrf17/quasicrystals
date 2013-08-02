#include "shader_util.h"

#include <fstream>
#include <streambuf>

namespace {

void GetShaderInfoLog(GLuint shader, std::string* debug) {
  if (debug == nullptr) {
    return;
  }
  int info_log_length;
  char* buffer;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    buffer = new char[info_log_length];
    glGetShaderInfoLog(shader, info_log_length, nullptr, buffer);
    debug->append(buffer);
    delete[] buffer;
  }
}

void GetProgramInfoLog(GLuint shader, std::string* debug) {
  if (debug == nullptr) {
    return;
  }
  int info_log_length;
  char* buffer;
  glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 0) {
    buffer = new char[info_log_length];
    glGetProgramInfoLog(shader, info_log_length, nullptr, buffer);
    debug->append(buffer);
    delete[] buffer;
  }
}

}  // namespace

bool ShaderUtil::BuildShader(const std::string& source,
                             const GLenum type,
                             GLuint* program,
                             std::string* debug) {

  GLuint f = glCreateShader(type);
  const char* source_cstr = source.c_str();
  glShaderSource(f, 1, &source_cstr, nullptr);

  GLint status;
  glCompileShader(f);
  glGetShaderiv(f, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GetShaderInfoLog(f, debug);
    return false;
  }

  *program = glCreateProgram();
  glAttachShader(*program, f);

  glLinkProgram(*program);
  glGetProgramiv(*program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GetProgramInfoLog(*program, debug);
    return false;
  }

  glUseProgram(*program);
  return true;
}

bool ShaderUtil::BuildShaderFromFile(const std::string& filename,
                                     const GLenum type,
                                     GLuint* program,
                                     std::string* debug) {
  std::ifstream infile(filename);
  std::string source((std::istreambuf_iterator<char>(infile)),
                     std::istreambuf_iterator<char>());
  return BuildShader(source, type, program, debug);
}
