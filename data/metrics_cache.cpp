#include "data/metrics_cache.h"
#include <time.h>

namespace {
  constexpr int HIST_N = 12;
  float    cpu_hist[HIST_N] = {0};
  uint32_t ts_hist [HIST_N] = {0};
  int      hist_len = 0;

  static String hhmm(uint32_t epoch) {
    time_t t = (time_t)epoch;
    struct tm tmv;
    gmtime_r(&t, &tmv);
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", tmv.tm_hour, tmv.tm_min);
    return String(buf);
  }
}

namespace cache {

void init() {
  hist_len = 0;
  for (int i=0;i<HIST_N;i++){ cpu_hist[i]=0; ts_hist[i]=0; }
}

void pushCpu(float pct, uint32_t ts) {
  if (hist_len < HIST_N) {
    cpu_hist[hist_len] = pct;
    ts_hist [hist_len] = ts;
    hist_len++;
  } else {
    for (int i=1;i<HIST_N;i++){ cpu_hist[i-1]=cpu_hist[i]; ts_hist[i-1]=ts_hist[i]; }
    cpu_hist[HIST_N-1] = pct;
    ts_hist [HIST_N-1] = ts;
  }
}

BarChartSeries buildCpuSeries() {
  BarChartSeries s;
  s.name = F("CPU %");
  s.yMin = 0.0f;
  s.yMax = 100.0f;

  s.categories.reserve(hist_len);
  s.values.reserve(hist_len);
  for (int i=0;i<hist_len;i++) {
    s.categories.push_back((i % 2 == 0) ? hhmm(ts_hist[i]) : String(""));
    s.values.push_back(cpu_hist[i]);
  }
  return s;
}

String lastUpdatedLabel() {
  if (hist_len == 0) return String();
  return String(F("Updated: ")) + hhmm(ts_hist[hist_len-1]);
}

} // namespace cache
