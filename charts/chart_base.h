#pragma once
#include <Arduino.h>
#include "gfx/igraphics.h"

#ifndef TFT_BLACK
  #define TFT_BLACK 0x0000
#endif
#ifndef TFT_WHITE
  #define TFT_WHITE 0xFFFF
#endif

struct ChartStyle {
  uint16_t bg=TFT_WHITE, fg=TFT_BLACK, grid=TFT_BLACK, accent=TFT_BLACK;
  uint8_t  pad=8, tick=3;
};

class Chart {
public:
  virtual ~Chart() {}
  // `model` can be any chart-specific struct (e.g., BarChartSeries)
  virtual void plot(IGraphics& g, const Rect& r, const void* model,
                    const String& xLabel, const String& yLabel,
                    const String& updated, const ChartStyle& s) = 0;

  void draw(IGraphics& g, const Rect& r, const void* model,
            const String& xLabel, const String& yLabel,
            const String& updated, const ChartStyle& s) { plot(g,r,model,xLabel,yLabel,updated,s); }
};
