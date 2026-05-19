#include "charts/chart_base.h"
#include "charts/models.h"
#include "gfx/igraphics.h"

class BarChart : public Chart {
public:
  void plot(IGraphics& g, const Rect& r, const void* model,
            const String& xLabel, const String& yLabel, const String& lastUpdated,
            const ChartStyle& st) override
  {
    // Model
    if (!model) return;
    const BarChartSeries* series = static_cast<const BarChartSeries*>(model);
    const BarChartSeries& data = *series;

    // Background/frame
    g.fillRect(r.x, r.y, r.w, r.h, st.bg);
    g.drawRect(r.x, r.y, r.w, r.h, st.fg);

    const int16_t th  = g.textHeight();
    const int16_t pad = st.pad;

    // Reserve space for labels
    const int16_t xLabelH       = th + 2;
    const int16_t yLabelW       = g.textWidth(yLabel) + 4;
    const int16_t bottomLabelH  = th + 4;  // for categories or "updated"

    // Plot area
    const int16_t px = r.x + pad + yLabelW + 4;
    const int16_t py = r.y + pad;
    const int16_t pw = r.w - (pad*2 + yLabelW + 4);
    const int16_t ph = r.h - (pad*2 + xLabelH + bottomLabelH);

    if (pw <= 10 || ph <= 10) return;

    // Axes
    g.drawLine(px,      py,      px, py + ph, st.fg); // Y
    g.drawLine(px, py + ph, px + pw, py + ph, st.fg);         // X

    // Scale
    float vmin = data.yMin;
    float vmax = (isnan(data.yMax) ? vmin : data.yMax);
    for (float v : data.values) {
      if (isnan(v)) continue;
      if (isnan(data.yMax)) vmax = (vmax==vmin) ? v : max(vmax, v);
      vmin = min(vmin, v);
    }
    if (vmax <= vmin) vmax = vmin + 1.0f;

    auto y2p = [&](float v)->int16_t {
      float t = (v - vmin) / (vmax - vmin);
      if (t < 0) t = 0; if (t > 1) t = 1;
      return py + ph - int16_t(t * ph + 0.5f);
    };

    // Grid (5 horizontal lines)
    for (int i=0;i<=5;i++){
      int16_t gy = py + (ph * i)/5;
      g.drawLine(px, gy, px+pw, gy, st.grid);
    }

    // Y ticks (2-3 labels)
    for (int i=0;i<=4;i+=2){
      float vy = vmin + (vmax - vmin) * (float(i)/4.0f);
      int16_t yy = y2p(vy);
      g.drawLine(px - st.tick, yy, px, yy, st.fg);
      String lab = String(vy, (fabs(vmax-vmin) < 5) ? 2 : 0);
      int16_t tw = g.textWidth(lab);
      g.drawText(px - st.tick - 2 - tw, yy + th/2, lab, st.fg);
    }

    // Bars
    const size_t n = data.values.size();
    const int16_t gap = 2; // px between bars
    if (n > 0) {
      const int16_t barSpace = max<int16_t>(1, pw / int16_t(n));
      const int16_t barW     = max<int16_t>(1, barSpace - gap);
      for (size_t i=0;i<n;i++){
        float v = data.values[i];
        int16_t x = px + i*barSpace + gap/2;
        int16_t y = y2p(max(v, vmin));
        g.fillRect(x, y, barW, (py+ph - y), st.accent);

        // category label (truncate if too wide)
        if (i < data.categories.size()) {
          String c = data.categories[i];
          if (g.textWidth(c) > barSpace-2) {
            if (c.length() > 3) c = c.substring(0,3) + ".";
          }
          g.drawText(x, py+ph + th + 2, c, st.fg);
        }
      }
    }

    // Axis labels
    if (xLabel.length()) {
      int16_t tw = g.textWidth(xLabel);
      g.drawText(px + (pw - tw)/2, py + ph + th + 2, xLabel, st.fg);
    }
    if (yLabel.length()) {
      g.drawText(r.x + pad, py - 2, yLabel, st.fg);
    }

    // Last updated
    if (lastUpdated.length()) {
      int16_t tw = g.textWidth(lastUpdated);
      g.drawText(r.x + r.w - pad - tw, r.y + r.h - pad, lastUpdated, st.fg);
    }
  }
};

// Factory
Chart* MakeBarChart() { return new BarChart(); }
