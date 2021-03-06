#ifndef FLAME_INCLUDED
#define FLAME_INCLUDED

#include "main.h"
#include "context.h"

#include <vector>


/*
 *     OO   OO   OO
 *     OO   OO   OO
 *
 *       .. .. ..
 *       OO OO OO
 *
 *        O O O
 *
 *         ...
 *
 * Blocks spin freely and uniquely.
 * Blocks disappear as they travel.
 * Blocks keep their position relative to the width.
 *    50% position would be in the middle and stay there.
 *    0% position is to the left, but will be to the left as the width of
 *       the line shrinks.
 *
 */

class Flame {
public:
  /*
   * Constructs a Flame engine.
   */
  Flame(float x, float y, float z);

  void setRotationX(float rotation);
  void setRotationY(float rotation);
  void setRotationZ(float rotation);

  void setColor(int color);

  void update(float elapsed);
  void draw(Context* context);

private:
  struct BlockInfo {
    float life;       // Distance travelled from ship engine
    float rotvx;      // Rotation X velocity
    float rotvy;      // Rotation Y velocity
    float rotvz;      // Rotation Z velocity
    float rotx;       // Rotation X
    float roty;       // Rotation Y
    float rotz;       // Rotation Z
    float size;       // Scale
    float position;   // Position on the line as a percentage
    float color;      // Block index

    float start_x;
    float start_y;
    float start_z;

    float base_rot_y;
    float base_rot_z;
    float base_rot_x;
  };

  void _addBlock(float position);
  void _updateBlock(float elapsed, BlockInfo& block);
  void _drawBlock(Context* context, BlockInfo& block);

  std::vector<BlockInfo> _blocks;

  unsigned int _rate;
  float        _min_freq;

  float        _elapsed;

  float _position_x;
  float _position_y;
  float _position_z;

  float _rotation_z;
  float _rotation_y;
  float _rotation_x;

  int   _color;
};

#endif
