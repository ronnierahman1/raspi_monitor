#pragma once
#include <Arduino.h>

// Returns true and fills payload on success; else false and `err`.
bool httpGetMetrics(String& payload, String& err);
