#pragma once
#include "gfx/igraphics.h"
#include <Arduino.h>

struct CardStyle {
  uint16_t frame=0x0000, bg=0xFFFF, text=0x0000;
  uint8_t pad=6;
};

class Card {
public:
  virtual ~Card() {}
  virtual void draw(IGraphics& g, const Rect& r, const CardStyle& s) = 0;
};
