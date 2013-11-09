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
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;
  std::vector<GLushort>  elements;

  std::ifstream in(filename, std::ios::in);
  if (!in) { std::cerr << "Cannot open " << filename << std::endl; exit(1); }

  std::string line;
  while (getline(in, line)) {
    if (line.substr(0,2) == "v ") {
      std::istringstream s(line.substr(2));
      glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
      vertices.push_back(v);
    }
    else if (line.substr(0, 3) == "vn ") {
      std::istringstream s(line.substr(3));
      glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
      normals.push_back(v);
    }
    else if (line.substr(0, 3) == "vt ") {
      std::istringstream s(line.substr(3));
      glm::vec2 v; s >> v.x; s >> v.y;
      texcoords.push_back(v);
    }
    else if (line.substr(0, 2) == "f ") {
      std::istringstream s(line.substr(2));
      GLushort a,b,c;

      s >> a; s.get(); s >> b; s.get(); s >> c;
      a--; b--; c--;
      elements.push_back(a); elements.push_back(b); elements.push_back(c);

      s >> a; s.get(); s >> b; s.get(); s >> c;
      a--; b--; c--;
      elements.push_back(a); elements.push_back(b); elements.push_back(c);

      s >> a; s.get(); s >> b; s.get(); s >> c;
      a--; b--; c--;
      elements.push_back(a); elements.push_back(b); elements.push_back(c);
    }
    else if (line[0] == '#') { /* ignoring this line */ }
    else { /* ignoring this line */ }
  }

  // Interleave
  float* data = new float[(elements.size()/3) * 8];
  size_t k = 0;
  for(size_t i = 0; i < elements.size(); i+=3) {
    data[k+0] = vertices[elements[i+0]].x;
    data[k+1] = vertices[elements[i+0]].y;
    data[k+2] = vertices[elements[i+0]].z;
    data[k+3] = normals[elements[i+2]].x;
    data[k+4] = normals[elements[i+2]].y;
    data[k+5] = normals[elements[i+2]].z;
    data[k+6] = texcoords[elements[i+1]].x;
    data[k+7] = texcoords[elements[i+1]].y;
    k+=8;
  }

  unsigned short* element_data = new unsigned short[elements.size()/3];
  for(size_t i = 0; i < elements.size()/3; i++) {
    element_data[i] = i;
  }

  _construct(data,         elements.size() / 3 * 8,
             element_data, elements.size() / 3);

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
