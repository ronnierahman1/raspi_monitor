#include "transport/http_client.h"
#include "config/config.h"
#include <HTTPClient.h>

bool httpGetMetrics(String& payload, String& err) {
  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("X-Token", TOKEN);
  const int code = http.GET();
  if (code == 200) {
    payload = http.getString();
    http.end();
    return true;
  }
  err = String("HTTP ") + code;
  http.end();
  return false;
}
