#pragma once

// Wi-Fi credentials and secret token — loaded from secrets.h (not committed)
// Copy config/secrets.h.example to config/secrets.h and fill in your values.
#include "secrets.h"

#define API_URL     "http://192.168.1.1:8088/metrics"

// Refresh cadence (no deep sleep)
#define REFRESH_MS  (60UL * 1000UL)   // 1 minute