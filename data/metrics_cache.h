#pragma once
#include <Arduino.h>
#include "charts/models.h"     // BarChartSeries

namespace cache {

// initialize buffers (optional, currently static state local to .cpp)
void init();

// push one sample
void pushCpu(float pct, uint32_t ts);

// build a BarChartSeries from history (with light x labels)
BarChartSeries buildCpuSeries();

// "Updated: HH:MM" built from the last timestamp; empty if no data yet
String lastUpdatedLabel();

} // namespace cache
