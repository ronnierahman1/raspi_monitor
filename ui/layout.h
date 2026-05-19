#pragma once
#include "gfx/igraphics.h"
#include "globals.h"

struct Layout {
  // define all rectangles used in the dashboard
  Rect cpuChart;
  Rect cpuTemp;
  Rect cpuUtil;
  Rect filesystemsBar;
  Rect fsTable;
  Rect loadAvg;
  Rect netTable;
  Rect netBadge;
  Rect servicesTable;
  Rect servicesBadges;
  Rect systemInfo;
  Rect smart;
  Rect sensors;
  Rect footer;
  Rect title;
  Rect chart;
   // 2×2 quarters
  Rect q1; // top-left
  Rect q2; // top-right
  Rect q3; // bottom-left
  Rect q4; // bottom-right


  static Layout make(int16_t W, int16_t H) {
    // 3 columns example
    const int16_t gap = 8;
    const int16_t colW = (W - (gap*3)) / 2;
    const int16_t rowH = (H - (gap*3)) / 2;
    const int16_t x1 = gap;
    const int16_t x2 = gap*2 + colW;
    int16_t x3 = gap*3 + colW*2;
    int16_t y = gap;
    const int16_t y1 = gap;
    const int16_t y2 = gap*2 + rowH;



    Layout L{};
    L.cpuChart     = {x1, y, colW, rowH};      L.fsTable   = {x2, y, colW, rowH};
    L.netTable     = {x3, y, colW, rowH};      y += rowH + gap;
    L.cpuTemp      = {x1, y, colW, rowH};      L.loadAvg   = {x2, y, colW, rowH};
    L.servicesTable= {x3, y, colW, rowH};      y += rowH + gap;
    L.cpuUtil      = {x1, y, colW, rowH};      L.systemInfo= {x2, y, colW, rowH};
    L.smart        = {x3, y, colW, rowH};      y += rowH + gap;
    L.filesystemsBar={x1, y, colW, rowH};      L.netBadge  = {x2, y, colW, rowH};
    L.sensors      = {x3, y, colW, rowH};      y += rowH + gap;
    L.footer       = {gap, y, W-gap*2, 28};
    L.title       = {gap, y, W-gap*2, 16};
    L.chart       = {gap, 24, W-gap*2, H-24-gap-8};  
    L.q1 = {x1, y1, colW, rowH};
    L.q2 = {x2, y1, colW, rowH};
    L.q3 = {x1, y2, colW, rowH};
    L.q4 = {x2, y2, colW, rowH};  
    return L;
  }
};
