#ifndef MESH_INCLUDED
#define MESH_INCLUDED

#include "main.h"
#include "context.h"

#include "glm/glm.hpp"

class Mesh {
public:
  /*
   * Constructs a Mesh from the given .obj file.
   */
  Mesh(const char* filename);

  /*
   * Constructs a Mesh with the given data.
   */
  Mesh(const float* data,              size_t data_count,
       const unsigned short* elements, size_t elements_count);

  /*
   * Destructs.
   */
  ~Mesh();

  /*
   * Draws the mesh with the given model matrix.
   */
  void draw(Context* context, glm::mat4& model);

  /*
   * Draws a subset of the vertices in this mesh.
   */
  void drawSubset(Context* context,
                  glm::mat4& model,
                  size_t start,
                  size_t count);

private:
  void _construct(const float* data,              size_t data_count,
                  const unsigned short* elements, size_t elements_count);

  GLuint _vbo_data;
  GLuint _vbo_elements;
  GLuint _count;
};

#endif
