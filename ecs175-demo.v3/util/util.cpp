//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#include "util.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fstream>
#include <string>

//---------------------------------------------------------------------------------------
// error check helper from EPFL ICG class
static inline const char*
ErrorString(GLenum error)
{
  const char* msg;
  switch (error) {
#define Case(Token) \
  case Token:       \
    msg = #Token;   \
    break;
    Case(GL_INVALID_ENUM);
    Case(GL_INVALID_VALUE);
    Case(GL_INVALID_OPERATION);
    Case(GL_INVALID_FRAMEBUFFER_OPERATION);
    Case(GL_NO_ERROR);
    Case(GL_OUT_OF_MEMORY);
#undef Case
  }
  return msg;
}

void
_glCheckError(const char* file, int line, const char* comment)
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "ERROR: %s (file %s, line %i: %s).\n", comment, file, line,
            ErrorString(error));
  }
}

//---------------------------------------------------------------------------------------

void
SaveJPG(const std::string& fname, std::vector<uint8_t>& fb, int w, int h)
{
  const size_t nchannel = fb.size() / ((size_t)w * (size_t)h);
  if (nchannel == 3) {
    stbi_write_jpg(fname.c_str(), w, h, 3, fb.data(), 100);
  }
  else if (nchannel == 4) {
    const int& width = w;
    const int& height = h;
    uint8_t* pixels = new uint8_t[width * height * 3];
    int index = 0;
    for (int j = height - 1; j >= 0; --j) {
      for (int i = 0; i < width; ++i) {
        int ir = int(fb[4 * (i + j * width) + 0]);
        int ig = int(fb[4 * (i + j * width) + 1]);
        int ib = int(fb[4 * (i + j * width) + 2]);
        pixels[index++] = ir;
        pixels[index++] = ig;
        pixels[index++] = ib;
      }
    }
    stbi_write_jpg(fname.c_str(), width, height, 3, pixels, 100);
    delete[] pixels;
  }
  else {
    throw std::runtime_error("Unknown image type");
  }
}

void
ReadFrame(GLFWwindow* window, std::vector<uint8_t>& buffer, int w, int h)
{
  const size_t nchannel = buffer.size() / ((size_t)w * (size_t)h);
  assert(nchannel == 3);

  // reading from the default framebuffer
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &(buffer[0]));
  check_error_gl("Save a frame");
}

//---------------------------------------------------------------------------------------

void
CheckShaderCompilationLog(GLuint shader, const std::string& fname)
{
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    // Provide the infolog in whatever manor you deem best.
    // Exit with failure.
    glDeleteShader(shader); // Don't leak the shader.
    // show the message
    std::cerr << "compilation error for shader: " << fname << std::endl
              << errorLog.data() << std::endl;
  }
}

static const char*
ReadShaderFile(const char* fname)
{
  std::ifstream file(fname, std::ios::binary | std::ios::ate | std::ios::in);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  char* buffer = new char[size + 1];
  buffer[size] = '\0';
  if (!file.read(const_cast<char*>(buffer), size)) {
    fprintf(stderr, "Error: Cannot read file %s\n", fname);
    exit(-1);
  }
  return buffer;
}

GLuint
LoadProgram_FromFiles(const char* vshader_fname, const char* fshader_fname)
{
  fprintf(stdout, "[shader] reading vertex shader file %s\n", vshader_fname);
  fprintf(stdout, "[shader] reading fragment shader file %s\n", fshader_fname);
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  {
    const char* vshader_text = ReadShaderFile(vshader_fname);
    glShaderSource(vshader, 1, &vshader_text, NULL);
    glCompileShader(vshader);
    CheckShaderCompilationLog(vshader, vshader_fname);
    check_error_gl("Compile Vertex Shaders");
  }
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  {
    const char* fshader_text = ReadShaderFile(fshader_fname);
    glShaderSource(fshader, 1, &fshader_text, NULL);
    glCompileShader(fshader);
    CheckShaderCompilationLog(fshader, fshader_fname);
    check_error_gl("Compile Fragment Shaders");
  }
  GLuint program = glCreateProgram();
  if (glCreateProgram == 0)
    throw std::runtime_error("wrong program");
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  check_error_gl("Compile Shaders: Attach");
  glLinkProgram(program);
  check_error_gl("Compile Shaders: Link");
  glUseProgram(program);
  check_error_gl("Compile Shaders: Final");
  return program;
}

GLuint
LoadProgram_FromEmbededTexts(const char* vshader_text, const char* fshader_text)
{
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  {
    glShaderSource(vshader, 1, &vshader_text, NULL);
    glCompileShader(vshader);
    CheckShaderCompilationLog(vshader, "embeded vertex shader");
    check_error_gl("Compile Vertex Shaders");
  }
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  {
    glShaderSource(fshader, 1, &fshader_text, NULL);
    glCompileShader(fshader);
    CheckShaderCompilationLog(fshader, "embeded fragment shader");
    check_error_gl("Compile Fragment Shaders");
  }
  GLuint program = glCreateProgram();
  if (glCreateProgram == 0)
    throw std::runtime_error("wrong program");
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  check_error_gl("Compile Shaders: Attach");
  glLinkProgram(program);
  check_error_gl("Compile Shaders: Link");
  glUseProgram(program);
  check_error_gl("Compile Shaders: Final");
  return program;
}
