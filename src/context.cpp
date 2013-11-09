#include "context.h"

#include <vector>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

static
void gl_check_errors(const char* msg) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    const char* errorString;
    switch ( error ) {
      case GL_INVALID_ENUM: errorString = "invalid enumerant"; break;
      case GL_INVALID_VALUE: errorString = "invalid value"; break;
      case GL_INVALID_OPERATION: errorString = "invalid operation"; break;
      case GL_STACK_OVERFLOW: errorString = "stack overflow"; break;
      case GL_STACK_UNDERFLOW: errorString = "stack underflow"; break;
      case GL_OUT_OF_MEMORY: errorString = "out of memory"; break;
      case GL_TABLE_TOO_LARGE: errorString = "table too large"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "invalid framebuffer operation"; break;
      default: errorString = "unknown GL error"; break;
    }
    fprintf(stderr, "GL Error: %s: %s\n", msg, errorString);
  }
}

static
const char* frag_shader_code[] = {
  "#ifdef GL_ES\n"
  "precision highp float;\n"
  "#endif\n"
  "\n"
  "varying vec3 Normal;\n"
  "varying vec2 Texcoord;\n"
  "\n"
  "uniform sampler2D tex;\n"
  "uniform float opacity;\n"
  "\n"
  "void main() {\n"
  "  gl_FragColor = texture2D(tex, Texcoord) * vec4(1.0, 1.0, 1.0, opacity);\n"
  "}"
};

static
const char* vertex_shader_code[] = {
  "#version 100\n"
  "\n"
  "attribute vec3 normal;\n"
  "attribute vec3 position;\n"
  "attribute vec2 texcoord;\n"
  "\n"
  "varying vec2 Texcoord;\n"
  "varying vec3 Normal;\n"
  "varying vec3 Position;\n"
  "varying float Opacity;\n"
  "\n"
  "uniform mat4 model;\n"
  "uniform mat4 view;\n"
  "uniform mat4 proj;\n"
  "\n"
  "uniform vec3 camera;\n"
  "\n"
  "void main() {\n"
  "  Texcoord = texcoord;\n"
  "  Normal = (model * vec4(normal, 1.0)).xyz;\n"
  "  Position = (model * vec4(position, 1.0)).xyz;\n"
  "\n"
  "  gl_Position = proj * view * model * vec4(position, 1.0);\n"
  "}"
};

Context::Context()
  : _opacity(1.0f),
    _id(0) {
  /* Generate program */
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader   = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(vertex_shader, 1, vertex_shader_code, NULL);
  glCompileShader(vertex_shader);

  GLint result = GL_FALSE;
  int infoLogLength;

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
  if (result != GL_TRUE) {
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> vertex_error_msg(infoLogLength);
    glGetShaderInfoLog(vertex_shader, infoLogLength, NULL, &vertex_error_msg[0]);
    fprintf(stdout, "%s\n", &vertex_error_msg[0]);
  }

  glShaderSource(frag_shader, 1, frag_shader_code, NULL);
  glCompileShader(frag_shader);

  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &result);
  if (result != GL_TRUE) {
    glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> frag_error_msg(infoLogLength);
    glGetShaderInfoLog(frag_shader, infoLogLength, NULL, &frag_error_msg[0]);
    fprintf(stdout, "%s\n", &frag_error_msg[0]);
  }

  _program = glCreateProgram();
  glAttachShader(_program, vertex_shader);
  glAttachShader(_program, frag_shader);
  glBindAttribLocation(_program, 0, "position");
  glLinkProgram(_program);
  gl_check_errors("glLinkProgram");

  glGetProgramiv(_program, GL_LINK_STATUS, &result);
  if (result != GL_TRUE) {
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> program_error_msg(infoLogLength);
    glGetProgramInfoLog(_program, infoLogLength, NULL, &program_error_msg[0]);
    fprintf(stdout, "%s\n", &program_error_msg[0]);
  }

  glUseProgram(_program);
  gl_check_errors("glUseProgram");

  glDeleteShader(vertex_shader);
  glDeleteShader(frag_shader);
  gl_check_errors("glDeleteShader");

  /* Attach/describe uniforms */
  _model_uniform = glGetUniformLocation(_program, "model");
  _view_uniform = glGetUniformLocation(_program, "view");
  _projection_uniform = glGetUniformLocation(_program, "proj");

  GLuint tex_uniform = glGetUniformLocation(_program, "tex");
  _opacity_uniform = glGetUniformLocation(_program, "opacity");
  gl_check_errors("glGetUniformLocation");

  GLint posAttrib = glGetAttribLocation(_program, "position");
  gl_check_errors("glGetAttribLocation position");

  glEnableVertexAttribArray(posAttrib);
  gl_check_errors("glEnableVertexAttribPointer position");

  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                        (GLsizei)(8 * sizeof(float)),
                        (const GLvoid*)(size_t)(0 * sizeof(float)));
  gl_check_errors("glVertexAttribPointer position");

  posAttrib = glGetAttribLocation(_program, "normal");
  gl_check_errors("glGetAttribLocation normal");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer normal");

    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(3 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer normal");
  }

  posAttrib = glGetAttribLocation(_program, "texcoord");
  gl_check_errors("glGetAttribLocation texcoord");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer texcoord");

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(6 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer texcoord");
  }

  /* set up perspective */
  _perspective  = glm::perspective(40.0f, (float)WIDTH/(float)HEIGHT, 1.0f, 200.0f);
  _orthographic = glm::ortho(-(float)WIDTH  / 2.0f, (float)WIDTH  / 2.0f,
                             -(float)HEIGHT / 2.0f, (float)HEIGHT / 2.0f);
  glUniformMatrix4fv(_projection_uniform, 1, GL_FALSE, &_perspective[0][0]);
  gl_check_errors("glUniformMatrix4fv perspective");

  /* set up view */
  _view = glm::lookAt(glm::vec3(0.0f, 0.0f, 21.5f),
                      glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 1.0f, 0.0));
  _viewOrtho = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0));
  glUniformMatrix4fv(_view_uniform, 1, GL_FALSE, &_view[0][0]);
  gl_check_errors("glUniformMatrix4fv view");

  glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  glUniformMatrix4fv(_model_uniform, 1, GL_FALSE, &model[0][0]);
  gl_check_errors("glUniformMatrix4fv model");

  glUniform1i(tex_uniform, 0);
  gl_check_errors("glUniform1i tex");

  glUniform1f(_opacity_uniform, _opacity);
  gl_check_errors("glUniform1i opacity");
}

void Context::usePerspective() {
  _in_perspective_mode = true;
}

void Context::useOrthographic() {
  _in_perspective_mode = false;
}

void Context::establish(int id) {
  if (_id == id) {
    return;
  }

  _id = id;

  /* Attach/describe uniforms */
  GLuint tex_uniform = glGetUniformLocation(_program, "tex");
  gl_check_errors("glGetUniformLocation");

  GLint posAttrib = glGetAttribLocation(_program, "position");
  gl_check_errors("glGetAttribLocation position");

  glEnableVertexAttribArray(posAttrib);
  gl_check_errors("glEnableVertexAttribPointer position");

  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                        (GLsizei)(8 * sizeof(float)),
                        (const GLvoid*)(size_t)(0 * sizeof(float)));
  gl_check_errors("glVertexAttribPointer position");

  posAttrib = glGetAttribLocation(_program, "normal");
  gl_check_errors("glGetAttribLocation normal");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer normal");

    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(3 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer normal");
  }

  posAttrib = glGetAttribLocation(_program, "texcoord");
  gl_check_errors("glGetAttribLocation texcoord");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer texcoord");

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(6 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer texcoord");
  }

  /* set up perspective/view */
  if (_in_perspective_mode) {
    glUniformMatrix4fv(_projection_uniform, 1, GL_FALSE, &_perspective[0][0]);
    gl_check_errors("glUniformMatrix4fv perspective");

    glUniformMatrix4fv(_view_uniform, 1, GL_FALSE, &_view[0][0]);
    gl_check_errors("glUniformMatrix4fv view");
  }
  else {
    glUniformMatrix4fv(_projection_uniform, 1, GL_FALSE, &_orthographic[0][0]);
    gl_check_errors("glUniformMatrix4fv orthographic");

    glUniformMatrix4fv(_view_uniform, 1, GL_FALSE, &_viewOrtho[0][0]);
    gl_check_errors("glUniformMatrix4fv viewOrtho");
  }

  glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  glUniformMatrix4fv(_model_uniform, 1, GL_FALSE, &model[0][0]);
  gl_check_errors("glUniformMatrix4fv model");

  glUniform1i(tex_uniform, 0);
  gl_check_errors("glUniform1i tex");

  glUniform1f(_opacity_uniform, _opacity);
  gl_check_errors("glUniform1i opacity");
}

void Context::setModel(glm::mat4& model) {
  glUniformMatrix4fv(_model_uniform, 1, GL_FALSE, &model[0][0]);
  gl_check_errors("glUniformMatrix4fv model");
}

void Context::setOpacity(float opacity) {
  _opacity = opacity;

  glUniform1f(_opacity_uniform, _opacity);
  gl_check_errors("glUniformMatrix4fv opacity");
}
