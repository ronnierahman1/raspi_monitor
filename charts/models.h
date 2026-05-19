#pragma once
#include <Arduino.h>
#include <vector>

// one-series bar chart model (extend here for line/pie later)
struct BarChartSeries {
  String name;
  std::vector<String> categories;  // x labels
  std::vector<float>  values;      // y values
  float yMin = 0.0f;
  float yMax = NAN;                // NaN => auto-scale
};
