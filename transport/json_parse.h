#pragma once
#include <Arduino.h>
#include "transport/json_model.h"

// Parse just the fields we need; robust to missing keys
bool parseMetricsLite(const String& json, MetricsLite& out, String& err);
