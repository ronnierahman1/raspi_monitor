#include "ui/app.h"
#include "ui/layout.h"
#include "globals.h"           
#include "config/config.h"     
#include "device/display.h"
#include "device/wifi.h"
#include "transport/http_client.h"
#include "transport/json_parse.h"
#include <ArduinoJson.h>
#include "data/metrics_cache.h"

#include "gfx/gfx_adapter.h"
#include "gfx/igraphics.h"
#include "charts/chart_base.h"
#include "charts/barchart.cpp"
#include "charts/models.h"

extern Chart* MakeBarChart();          // implemented in your charts/barchart.cpp

static GfxAdapter<EPaper> GFX(epaper);
static uint32_t lastTick = 0;

// ---- arc helpers (reusable) -----------------------------------------------

// Draws the light track (two thin outlines) for a semi-donut ring.
static void drawArcTrack(int16_t cx, int16_t cy, int16_t rInner, int16_t rOuter,
                         int startDeg, int endDeg) {
  auto polar = [](int16_t cx, int16_t cy, int16_t r, float deg)->std::pair<int16_t,int16_t>{
    float rad = deg * 3.14159265f / 180.0f;
    return { int16_t(cx + r * cosf(rad)), int16_t(cy + r * sinf(rad)) };
  };
  for (int a = startDeg; a >= endDeg; --a) {
    auto [xo1, yo1] = polar(cx, cy, rInner + 1, a);
    auto [xi1, yi1] = polar(cx, cy, rInner, a);
    epaper.drawLine(xi1, yi1, xo1, yo1, TFT_BLACK);
  }
  for (int a = startDeg; a >= endDeg; --a) {
    auto [xo2, yo2] = polar(cx, cy, rOuter, a);
    auto [xi2, yi2] = polar(cx, cy, rOuter + 1, a);
    epaper.drawLine(xi2, yi2, xo2, yo2, TFT_BLACK);
  }
}

// Draws the value (filled arc) for a semi-donut ring, clamped to [minV,maxV].
static void drawArcValue(int16_t cx, int16_t cy, int16_t rInner, int16_t rOuter,
                         int startDeg, int endDeg,
                         float value, float minV, float maxV) {
  if (maxV <= minV) maxV = minV + 1.0f;
  if (isnan(value)) value = minV;
  float t = (value - minV) / (maxV - minV);
  if (t < 0) t = 0; if (t > 1) t = 1;

  const float span   = float(startDeg - endDeg);
  const float valDeg = endDeg + t * span;

  auto polar = [](int16_t cx, int16_t cy, int16_t r, float deg)->std::pair<int16_t,int16_t>{
    float rad = deg * 3.14159265f / 180.0f;
    return { int16_t(cx + r * cosf(rad)), int16_t(cy + r * sinf(rad)) };
  };

  for (int a = endDeg; a <= int(valDeg); ++a) {
    auto [xo, yo] = polar(cx, cy, rOuter, a);
    auto [xi, yi] = polar(cx, cy, rInner, a);
    epaper.drawLine(xi, yi, xo, yo, TFT_BLACK);
  }
  // end cap
  auto [xo, yo] = polar(cx, cy, rOuter, startDeg);
  auto [xi, yi] = polar(cx, cy, rInner, startDeg);
  epaper.drawLine(xi, yi, xo, yo, TFT_BLACK);
}

// ---- generic single-value gauge (uses the arc helpers) ---------------------

static void drawGauge(const Rect& area,
                      const __FlashStringHelper* title,
                      float value, float minV, float maxV,
                      const __FlashStringHelper* unit,
                      uint8_t digits = 1)
{
  // Frame + title
  epaper.drawRect(area.x, area.y, area.w, area.h, TFT_BLACK);
  epaper.setCursor(area.x + 6, area.y + 14);
  epaper.setTextColor(TFT_BLACK);
  epaper.print(title);

  // Geometry (same as your working version)
  const int16_t pad    = 8;
  const int16_t cx     = area.x + area.w/2;
  const int16_t cy     = area.y + area.h/2 + 8;
  const int16_t rOuter = min(area.w, area.h)/2 - pad;
  const int16_t rInner = rOuter - 12;
  const int startDeg   = 405;   // 225°+180°
  const int endDeg     = 135;   // -45°+180°

  // Track + value
  drawArcTrack(cx, cy, rInner, rOuter, startDeg, endDeg);
  drawArcValue (cx, cy, rInner, rOuter, startDeg, endDeg, value, minV, maxV);

  // Center readout
  char buf[32];
  dtostrf(value, 0, digits, buf);
  String s(buf); s += unit;
  int16_t tw = GFX.textWidth(s);
  int16_t th = GFX.textHeight();
  epaper.setCursor(cx - tw/2, cy - th/2);
  epaper.print(s);
}
// ---- end of arc/gauge helpers ----------------------------------------------

// 3-ring load average gauge. We normalize by CPU count so the
// arc spans 0..1.0 (i.e., 100%) is 1 runnable per CPU.
static void drawLoadGauge(const Rect& area, float l1, float l5, float l15) {
  epaper.drawRect(area.x, area.y, area.w, area.h, TFT_BLACK);
  epaper.setCursor(area.x + 6, area.y + 14);
  epaper.setTextColor(TFT_BLACK);
  epaper.print(F("Load Average"));

  // Geometry
  const int16_t pad    = 8;
  const int16_t cx     = area.x + area.w/2 - 40;
  const int16_t cy     = area.y + area.h/2 + 8;
  const int16_t R      = min(area.w, area.h)/2 - pad;
  const int16_t gap    = 4;       // gap between rings
  const int16_t thick  = 10;      // ring thickness
  const int startDeg   = 405;
  const int endDeg     = 135;

  // Normalization target (1.0 = fully utilized). If you want
  // to show raw load (not normalized), set norm=1.0.
  const float norm = 1.0f; // “per CPU” normalization would need the core count.

  // Outer = 1m, mid = 5m, inner = 15m
  struct Ring { float v; int16_t rIn, rOut; };
  Ring rings[3] = {
    { l1 / norm,   R - thick,     R          },           // outer
    { l5 / norm,   R - thick*2 - gap,  R - thick - gap }, // middle
    { l15 / norm,  R - thick*3 - gap*2, R - thick*2 - gap } // inner
  };

  for (auto& rg : rings) {
    drawArcTrack(cx, cy, rg.rIn, rg.rOut, startDeg, endDeg);
    // clamp to [0,1] for the arc mapping
    float vv = rg.v; if (vv < 0) vv = 0; if (vv > 1) vv = 1;
    drawArcValue(cx, cy, rg.rIn, rg.rOut, startDeg, endDeg, vv, 0.0f, 1.0f);
  }

  // Legend (right side)
  const int16_t baseX = area.x + area.w/2 + R  ; // tweak if needed
  int16_t y = area.y + area.h/2;                    // start a bit below mid
  auto legend = [&](const __FlashStringHelper* tag, float v){
    char buf[12]; dtostrf(v, 0, 2, buf);
    String s(buf);
    epaper.setCursor(baseX, y);
    epaper.print(s);
    y += GFX.textHeight();
    epaper.setCursor(baseX, y);
    epaper.print(tag);
    y += GFX.textHeight() + 4;
  };
  legend(F("1 min"),  l1);
  legend(F("5 min"),  l5);
  legend(F("15 min"), l15);
}

// end of drawLoadGauge()

static float g_lastCpuPct = NAN;

static bool fetchCpuUtilOnce() {
  String payload, err;
  if (!httpGetMetrics(payload, err)) return false;

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) return false;

  if (doc["cpu"]["usage_pct"].is<float>()) {
    g_lastCpuPct = doc["cpu"]["usage_pct"].as<float>();
    return true;
  }
  return false;
}


// Fetch 1-hour CPU usage history from the Pi (OMV RRD wrapper) and push into cache.
static bool fetchCpuHistory1h() {
  String payload, err;
  // Reuse your HTTP client; build the URL with query string.
  // If your httpGetMetrics() always hits /metrics, add httpGetRaw(url, payload, err) instead.
  // For now, we’ll call the same host configured in your http client and pass a full path:
  if (!httpGetMetrics(payload, err)) {  // If client can’t target arbitrary paths,
    return false;                       // we can create a sibling function httpGetPath("/rrd/cpu?period=1h", ...)
  }

  // Parse: {"period":"-1h","points":[{"ts":..., "value": 23.1}, ...], "cores": N}
  Serial.begin(115200);
  StaticJsonDocument<8192> doc;
  DeserializationError e = deserializeJson(doc, payload);
  if (e) return false;
  Serial.println(F("RRD fetch success"));
  Serial.println(doc.as<String>());
  JsonArray pts = doc["points"].as<JsonArray>();
  if (pts.isNull()) return false;

  // Clear then push newest up to ~12 samples (or all if you prefer)
  cache::init();
  // We’ll thin labels later in buildCpuSeries(); push every point here.
  for (JsonObject p : pts) {
    uint32_t ts = p["ts"] | 0u;
    float    v  = p["value"] | 0.0f;
    cache::pushCpu(v, ts);
  }
  Serial.end();
  return true;
}

static float g_lastTempC = NAN;

// Pulls a single snapshot from /metrics and caches g_lastTempC
static bool fetchCpuTempOnce() {
  String payload, err;
  if (!httpGetMetrics(payload, err)) return false;

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) return false;

  if (doc["cpu"]["temp_c"].is<float>()) {
    g_lastTempC = doc["cpu"]["temp_c"].as<float>();
    return true;
  }
  return false;
}


static float g_load1 = NAN, g_load5 = NAN, g_load15 = NAN;

static bool fetchLoadOnce() {
  String payload, err;
  if (!httpGetMetrics(payload, err)) return false;

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) return false;

  bool ok = false;
  if (doc["cpu"]["load1"].is<float>())  { g_load1  = doc["cpu"]["load1"].as<float>();  ok = true; }
  if (doc["cpu"]["load5"].is<float>())  { g_load5  = doc["cpu"]["load5"].as<float>();  ok = true; }
  if (doc["cpu"]["load15"].is<float>()) { g_load15 = doc["cpu"]["load15"].as<float>(); ok = true; }
  return ok;
}


static void drawCpuDashboard() {
  auto L = Layout::make(SCR_W, SCR_H);
    epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_BLACK);epaper.update();
    epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_WHITE);epaper.update();

  // Clear whole canvas
  epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_WHITE);epaper.update();

  // --- Q1: CPU (last 1h) bar chart (unchanged) ---
  {
    epaper.drawRect(L.q1.x, L.q1.y, L.q1.w, L.q1.h, TFT_BLACK);
    epaper.setCursor(L.q1.x + 6, L.q1.y + 4);
    epaper.setTextColor(TFT_BLACK);
    epaper.print(F("CPU (last 1h)"));

    Rect chartArea{ int16_t(L.q1.x + 4), int16_t(L.q1.y + 18),
                    int16_t(L.q1.w - 8), int16_t(L.q1.h - 22) };

    BarChartSeries series = cache::buildCpuSeries();
    ChartStyle style;
    const String xLabel = F("Time");
    const String yLabel = F("CPU (%)");
    const String updated = cache::lastUpdatedLabel();

    Chart* chart = MakeBarChart();
    chart->draw(GFX, chartArea, &series, xLabel, yLabel, updated, style);
    delete chart;
  }

  // --- Q2: CPU Temp gauge ---
    drawGauge(L.q2, F("CPU Temp"), g_lastTempC, 0.0f, 100.0f, F(" C"), 1);

  
  // --- Q3: CPU Utilization gauge ---
  //   epaper.drawRect(L.q3.x, L.q3.y, L.q3.w, L.q3.h, TFT_BLACK);

  drawGauge(L.q3, F("CPU Utilization"), g_lastCpuPct, 0.0f, 100.0f, F("%"), 1);

  // --- Q4: Load Average (3 rings) ---
  drawLoadGauge(L.q4, g_load1, g_load5, g_load15);


  epaper.setCursor(L.q4.x + 6, L.q4.y + 14); epaper.print(F("Chart 4 (coming soon)"));

  epaper.update();
}

static void drawMemoryDashboard()
{

}


void App::setup() {
  displayInit();
  cache::init();

  if (wifiConnect()) {
    // Prefer RRD (1h history) over the single-sample /metrics
    if (!fetchCpuHistory1h()) {
      // fallback: one snapshot from /metrics (keeps screen non-empty)
      String payload, err;
      if (httpGetMetrics(payload, err)) {
        MetricsLite m{};
        if (parseMetricsLite(payload, m, err)) {
          cache::pushCpu(m.cpu_pct, m.ts);
        }
      }
    }
    fetchCpuTempOnce();
    fetchCpuUtilOnce();
    fetchLoadOnce();
    wifiOff();
  }

  drawCpuDashboard();
  lastTick = millis();
}


int count=0;

void App::loop() {
    if (millis() - lastTick < REFRESH_MS) return;
    lastTick = millis();
    if(count >= 5) {
      count=0;
      epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_BLACK);epaper.update();
      epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_WHITE);epaper.update();
    }
    count++;

    if (!wifiConnect()) { /* tiny banner like you *****************************************had */ epaper.update(); return; }

    if (fetchCpuHistory1h() && fetchCpuTempOnce() && fetchCpuUtilOnce() && fetchLoadOnce()) {
        drawCpuDashboard();
    } else {
        // show an error banner at the bottom
        epaper.setCursor(8, SCR_H - 8);
        epaper.setTextColor(TFT_BLACK);
        epaper.print(F("RRD fetch error"));
        epaper.update();
    }

    wifiOff();
}

void deepClean(){
    epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_BLACK);
    epaper.fillRect(0, 0, SCR_W, SCR_H, TFT_WHITE);
}