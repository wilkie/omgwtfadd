#ifndef CONTEXT_INCLUDED
#define CONTEXT_INCLUDED

#include "main.h"

#include "glm/glm.hpp"

class Context {
public:
  /*
   * Creates a context for all video operations.
   */
  Context();

  /*
   * Toggles the perspective view.
   */
  void usePerspective();

  /*
   * Toggles the orthographic view.
   */
  void useOrthographic();

  /*
   * Establishes the gpu programs and matrices.
   */
  void establish();

  /*
   * Sets the model matrix for the next render.
   */
  void setModel(glm::mat4& model);

  /*
   * Sets the opacity for the next render.
   */
  void setOpacity(float opacity);

private:
  bool _in_perspective_mode;

  GLuint _model_uniform;
  GLuint _view_uniform;
  GLuint _projection_uniform;

  GLuint _opacity_uniform;

  float _opacity;

  glm::mat4 _perspective;
  glm::mat4 _view;
  glm::mat4 _orthographic;
  glm::mat4 _viewOrtho;

  GLuint _program;
};

#endif
