// Utility functions for loading OpenGL shaders.

#ifndef QUASICRYSTAL_SHADER_UTIL_H
#define QUASICRYSTAL_SHADER_UTIL_H

#include <string>

#include <GL/glew.h>

namespace graphics {

class ShaderUtil {
 public:
  // Given the source of a shader, compile it and link it into a new
  // program.  If compilation or linking fails and debug is not null,
  // then debug will contain any error messages.
  static bool BuildShader(const std::string& source,
                          const GLenum type,
                          GLuint* program,
                          std::string* debug = nullptr);

  // Given a path to a file with the source of a shader, compile and link
  // it into a new program.  If compilation or linking fails and debug is not
  // null, then debug will contain any error messages.
  static bool BuildShaderFromFile(const std::string& filename,
                                  const GLenum type,
                                  GLuint* program,
                                  std::string* debug = nullptr);

  // TODO(piotrf): functions for compiling and linking into an existing
  // program.
};

}  // namespace graphics

#endif
