#include "mesh.h"

#include <vector>
#include <ios>
#include <iostream>
#include <sstream>
#include <fstream>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

static void gl_check_errors(const char* msg) {
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

Mesh::Mesh(const char* filename) {
  std::vector<glm::vec4> vertices;
  std::vector<glm::vec3> normals;
  std::vector<GLushort>  elements;

  std::ifstream in(filename, std::ios::in);
  if (!in) { std::cerr << "Cannot open " << filename << std::endl; exit(1); }

  std::string line;
  while (getline(in, line)) {
    if (line.substr(0,2) == "v ") {
      std::istringstream s(line.substr(2));
      glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
      vertices.push_back(v);
    }  else if (line.substr(0,2) == "f ") {
      std::istringstream s(line.substr(2));
      GLushort a,b,c;
      s >> a; s >> b; s >> c;
      a--; b--; c--;
      elements.push_back(a); elements.push_back(b); elements.push_back(c);
    }
    else if (line[0] == '#') { /* ignoring this line */ }
    else { /* ignoring this line */ }
  }

  normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
  for (int i = 0; i < elements.size(); i+=3) {
    GLushort ia = elements[i];
    GLushort ib = elements[i+1];
    GLushort ic = elements[i+2];
    glm::vec3 normal = glm::normalize(glm::cross(
          glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
          glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
    normals[ia] = normals[ib] = normals[ic] = normal;
  }

  // Interleave
  float* data = new float[vertices.size() * 8];
  for(size_t i = 0; i < vertices.size(); i++) {
    data[i*8+0] = vertices[i].x;
    data[i*8+1] = vertices[i].y;
    data[i*8+2] = vertices[i].z;
    data[i*8+3] = normals[i].x;
    data[i*8+4] = normals[i].y;
    data[i*8+5] = normals[i].z;
    data[i*8+6] = 0.0;
    data[i*8+7] = 0.0;
  }

  unsigned short* element_data = new unsigned short[elements.size()];
  for(size_t i = 0; i < elements.size(); i++) {
    element_data[i] = elements[i];
  }

  _construct(data,         vertices.size() * 8,
             element_data, elements.size());

  printf("%d\n", _count);

  delete [] data;
  delete [] element_data;
}

Mesh::Mesh(const float* data, size_t data_count,
           const unsigned short* elements, size_t elements_count) {
  _construct(data,     data_count,
             elements, elements_count);
}

void Mesh::_construct(const float* data,              size_t data_count,
                      const unsigned short* elements, size_t elements_count) {
  glGenBuffers(1, &_vbo_data);
  glGenBuffers(1, &_vbo_elements);
  gl_check_errors("glGenBuffers");

  glBindBuffer(GL_ARRAY_BUFFER, _vbo_data);
  glBufferData(GL_ARRAY_BUFFER, data_count * sizeof(float), data, GL_STATIC_DRAW);
  gl_check_errors("glBufferData cube_data");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_elements);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements_count * sizeof(unsigned short), elements, GL_STATIC_DRAW);
  gl_check_errors("glBufferData cube_elements");

  _count = elements_count;
}

Mesh::~Mesh() {
  // TODO: Delete buffers
}

void Mesh::draw(Context* context, glm::mat4& model) {
  drawSubset(context, model, 0, _count);
}

void Mesh::drawSubset(Context* context, glm::mat4& model,
                      size_t start,     size_t count) {
  glBindBuffer(GL_ARRAY_BUFFER,         _vbo_data);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_elements);

  context->establish(_vbo_data);

  context->setModel(model);

  glDrawElements(GL_TRIANGLES,       // Render a list of triangles
                 count,              // Number of elements in the buffer
                 GL_UNSIGNED_SHORT,  // Elements are shorts
                 (GLvoid*)(start * sizeof(unsigned short))); // Start index
  gl_check_errors("glDrawElements");
}
