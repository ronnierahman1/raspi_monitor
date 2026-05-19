// gfx_adapter.h
#pragma once
#include "gfx/igraphics.h"
#include <Arduino.h>

// Example adapter for an Adafruit_GFX-like display.
// Replace 'DisplayT' with your type (e.g., EPaper, TFT_eSPI).
template <typename DisplayT>
class GfxAdapter : public IGraphics {
public:
  explicit GfxAdapter(DisplayT& d) : d_(d) {}

  void drawLine(int16_t x0,int16_t y0,int16_t x1,int16_t y1,uint16_t color) override {
    d_.drawLine(x0,y0,x1,y1,color);
  }
  void drawRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color) override {
    d_.drawRect(x,y,w,h,color);
  }
  void fillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color) override {
    d_.fillRect(x,y,w,h,color);
  }
  void drawText(int16_t x,int16_t y,const String& text,uint16_t color) override {
    d_.setTextColor(color);
    // If you have drawString(), prefer it; otherwise use setCursor()+print()
    #if defined(HAS_DRAWSTRING)
      d_.drawString(text, x, y);           // y may be baseline or top, depending on your driver
    #else
      d_.setCursor(x, y);
      d_.print(text);
    #endif
  }
  int16_t textWidth(const String& text) const override {
    // If your driver has textWidth(), use it.
    #if defined(HAS_TEXTWIDTH)
      return d_.textWidth(text);
    #else
      int16_t x1,y1,w,h;
      // Many GFX drivers support getTextBounds; if not, return a rough guess.
      if constexpr (requires { d_.getTextBounds(text.c_str(), 0, 0, &x1,&y1,&w,&h); }) {
        const_cast<DisplayT&>(d_).getTextBounds(text.c_str(), 0, 0, &x1,&y1,&w,&h);
        return w;
      } else {
        // fallback: 6px * len (built-in font), adjust if you use FreeFonts
        return 6 * text.length();
      }
    #endif
  }
  int16_t textHeight() const override {
    #if defined(HAS_FONTHEIGHT)
      return d_.fontHeight();
    #else
      int16_t x1,y1,w,h;
      if constexpr (requires { d_.getTextBounds("Hg", 0, 0, &x1,&y1,&w,&h); }) {
        const_cast<DisplayT&>(d_).getTextBounds("Hg", 0, 0, &x1,&y1,&w,&h);
        return h;
      } else {
        return 8; // built-in font height
      }
    #endif
  }

private:
  DisplayT& d_;
};
