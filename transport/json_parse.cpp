
#include "transport/json_parse.h"
#include <ArduinoJson.h>

bool parseMetricsLite(const String& json, MetricsLite& out, String& err) {
  StaticJsonDocument<4096> doc;               // adjust if your JSON grows
  auto e = deserializeJson(doc, json);
  if (e) { err = e.c_str(); return false; }

  out.cpu_pct = doc["cpu"]["usage_pct"] | 0.0f;
  out.ts      = doc["ts"] | 0u;

  return true;
}
