#include "flame.h"

#include "main.h"
#include "components.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// How fast the flames move
#define FLAME_SPEED 0.5f

// How fast the flame blocks shrink
#define FLAME_BURN  0.2f

Flame::Flame(float x, float y)
  : _position_x(x),
    _position_y(y) {
  _rate = 20;
  _min_freq = 0.03;

  _addBlock(0.0f);
  _addBlock(0.33f);
  _addBlock(0.66f);
  _addBlock(1.0f);
}

void Flame::update(float elapsed) {
  for (size_t i = 0; i < _blocks.size(); i++) {
    _updateBlock(elapsed, _blocks[i]);

    if (_blocks[i].size < 0 || _blocks[i].life < 0) {
      _blocks.erase(_blocks.begin() + i);
      i--;
    }
  }

  _elapsed += elapsed;

  while(_elapsed > _min_freq) {
    _elapsed -= _min_freq;
    _addBlock((float)(rand() % 1000) / 1000.0f);
  }
}

void Flame::draw(Context* context) {
  for (size_t i = 0; i < _blocks.size(); i++) {
    _drawBlock(context, _blocks[i]);
  }
}

void Flame::_drawBlock(Context* context, BlockInfo& block) {
  context->setOpacity(block.life);

  float _width    = 2.0f;
  float _length   = 7.0f;
  float _size     = 0.1;

  float width = block.life * _width;

  float position_x = (block.position * width) + _position_x - (width / 2.0f);
  float position_y = _position_y - _length * (1.0f - block.life);

  float scale    = _size * block.size;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(position_x, position_y, 0.0f));
  model = glm::scale(model, glm::vec3(scale, scale, scale));
  model = glm::rotate(model, block.rotx, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, block.roty, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, block.rotz, glm::vec3(0.0f, 0.0f, 1.0f));

  engine.useTexture(block.color);
  engine.drawCube(model);

  context->setOpacity(1.0f);
}

void Flame::_addBlock(float position) {
  BlockInfo bi;

  bi.position = position;
  bi.life = 1.0f;

  bi.rotvx = (float)(rand() % 360);
  bi.rotvy = (float)(rand() % 360);
  bi.rotvz = (float)(rand() % 360);

  bi.rotx = 0.0f;
  bi.roty = 0.0f;
  bi.rotz = 0.0f;

  bi.size = 1.0f;

  bi.color = 4; ///rand() % 8;

  _blocks.push_back(bi);
}

void Flame::_updateBlock(float elapsed, BlockInfo& block) {
  block.rotx += block.rotvx * elapsed;
  block.roty += block.rotvy * elapsed;
  block.rotz += block.rotvz * elapsed;

  block.life -= FLAME_SPEED * elapsed;
  block.size -= FLAME_BURN  * elapsed;
}
