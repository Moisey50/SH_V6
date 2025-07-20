#pragma once

#include <gadgets/gadbase.h>

enum RGB_STATE
{
  STATE_UNKNOWN = 0,
  STATE_R = 1,
  STATE_G = STATE_R << 1,
  STATE_B = STATE_G << 1,
  STATE_RGB = STATE_R | STATE_G | STATE_B
};

//typedef map<RGB_STATE, const CVideoFrame*> VideoFramesOrder;