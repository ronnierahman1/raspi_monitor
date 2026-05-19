#pragma once
#include <Arduino.h>

struct Rect { int16_t x, y, w, h; };

class IGraphics {
public:
  virtual ~IGraphics() {}
  virtual void drawLine(int16_t x0,int16_t y0,int16_t x1,int16_t y1,uint16_t color)=0;
  virtual void drawRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color)=0;
  virtual void fillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color)=0;
  virtual void drawText(int16_t x,int16_t y,const String& text,uint16_t color)=0; // y=baseline
  virtual int16_t textWidth(const String& text) const =0;
  virtual int16_t textHeight() const =0;
};
