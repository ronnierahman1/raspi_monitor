// #include <Arduino.h>
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include <esp_wifi.h>        // if available on your core
// // #include "driver.h"
// #include "config/config.h"
// #include "globals.h"
// #include <Adafruit_GFX.h>
// #include <TFT_eSPI.h>      // Core graphics & driver
// #include "Fonts/fonts_data.h"    // Your FreeSans font externs
// #include "Fonts/FreeSans8pt7b.h"
// // #include "FreeSansBold12pt7b.h"


// extern "C" {
//   #include "esp_wifi.h"
//   #include "esp_event.h"
//   #include "esp_bt.h"
// }

// // ======= CONFIG =======
// EPaper epaper;
// const char* URL = "http://192.168.1.1:8088/metrics";
// // const char* TOKEN = "set-a-strong-token";
// const uint32_t SLEEP_MINUTES = 5;   // adjust to your battery needs
// // ======================
// // Epaper epaper;
// // TODO: include your ePaper header and init it in setup()
// void drawScreen(JsonDocument& doc) {
//   // Example: render a concise dashboard
//   // host / time
//   String host = doc["host"] | "-";
//   int uptime = doc["uptime_s"] | 0;
//   float t = doc["cpu"]["temp_c"] | NAN;
//   float l1 = doc["cpu"]["load1"] | NAN;
//   float mp = doc["mem"]["pct"] | NAN;
//   String ip = doc["net"]["ip"] | "-";

//   // // Clear and draw header
//   // epaper.fillScreen(TFT_WHITE);
//   // // draw header row
//   epaper.setTextSize(1);
//   epaper.setFreeFont(&FreeSans8pt7b);
//   epaper.drawString(String("Pi: ")+host+"  IP: "+ip, 10, 20);
//   epaper.drawString(String("Uptime: ")+(uptime/3600)+"h  CPU: "+String(t,1)+"°C  Load1: "+String(l1,2), 10, 40);
//   epaper.drawString(String("Mem: ")+String(mp,1)+"%", 10, 60);

//   // Disks (first 2)
//   JsonArray disks = doc["disk"].as<JsonArray>();
//   int y = 90;
//   int shown = 0;
//   for (JsonObject d : disks) {
//     if (shown++ >= 2) break;
//     String line = String(d["mnt"].as<const char*>()) + " " + String((float)d["pct"],1) + "% " +
//                   String((float)d["used_gb"],1) + "/" + String((float)d["total_gb"],1) + " GB";
//     epaper.drawString(line, 10, y);
//     y += 20;
//   }
// epaper.update();
//   // Services
//   JsonArray svcs = doc["services"].as<JsonArray>();
//   for (JsonObject s : svcs) {
//     bool active = s["active"] | false;
//     String line = String(s["name"].as<const char*>()) + (active ? " ✅" : " ❌");
//      epaper.drawString(line, 10, y);
//     y += 20;
//   }

//   // Logs (last few)
//   JsonArray logs = doc["logs"].as<JsonArray>();
//   epaper.drawString("Logs:", 10, y); y += 18;
//   int count = 0;
//   for (JsonVariant v : logs) {
//     if (count++ >= 6) break;          // fit on screen; adjust font/spacing
//     String ln = v.as<const char*>();
//     if (ln.length() > 64) ln = ln.substring(0,64) + "...";
//     epaper.drawString(ln, 10, y);
//     y += 20;
//   }

//   epaper.update();   // full refresh
// }
// void deepClean(EPaper& epaper) {
//   epaper.fillScreen(TFT_BLACK);
//   epaper.update();
//   epaper.fillScreen(TFT_WHITE);
//   epaper.update();
// }

// bool wifiConnect(uint32_t msTimeout=15000) {
//   epaper.setTextSize(2);
//   epaper.drawString("Connecting to WiFi...", 20, 40);
//   epaper.update();
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASS);

//   uint32_t t0 = millis();
//   while (WiFi.status() != WL_CONNECTED && millis() - t0 < msTimeout) delay(200);
//   return WiFi.status() == WL_CONNECTED;
// }

// void wifi_turnoff() {
//   WiFi.disconnect(true, true);
//   WiFi.mode(WIFI_OFF);
// #ifdef ESP_IDF_VERSION_MAJOR
//   esp_wifi_stop();
//   esp_wifi_deinit();
// #endif
// }

// void deepSleepMinutes(uint32_t m) {
//   // esp_sleep_enable_timer_wakeup((uint64_t)m * 60ULL * 1000000ULL);
//   // esp_deep_sleep_start();
// }
// void initialize_epaper()
// {
//   epaper.begin();
//   epaper.setRotation(4);
//   epaper.fillScreen(TFT_WHITE);
//   epaper.setTextColor(TFT_BLACK);
//   epaper.update();
// }

// void setup() {
//   initialize_epaper();

//   if (!wifiConnect()) {
//     epaper.drawString("WiFi failed. Sleeping...", 10, 20);
//     epaper.update();
//     deepSleepMinutes(SLEEP_MINUTES);
//   }

//   HTTPClient http;
//   http.begin(URL);
//   http.addHeader("X-Token", TOKEN);
//   int code = http.GET();

//   if (code == 200) {
//     String payload = http.getString();
//     StaticJsonDocument<4096> doc;  // size based on your JSON; tune!
//     DeserializationError err = deserializeJson(doc, payload);
//     if (!err) {
//       deepClean(epaper);
//       drawScreen(doc);
//     } else {
//       epaper.drawString("JSON parse error", 10, 20); epaper.update();
//     }
//   } else {
//     epaper.drawString("HTTP error: " + String(code), 10, 20); epaper.update();
//   }
//   http.end();
//   wifi_turnoff();
//   deepSleepMinutes(SLEEP_MINUTES);
//   epaper.update();

//   // epaper.fillScreen(TFT_WHITE);
//   // epaper.setTextColor(TFT_BLACK, TFT_WHITE);

//   // // 1) Built-in font sanity check
//   // epaper.setTextFont(2);
//   // epaper.setCursor(10, 30);
//   // epaper.print("Built-in OK");

//   // // 2) FreeFont check
//   // epaper.setFreeFont(&FreeSansBold12pt7b);
//   // epaper.setCursor(10, 80);      // baseline positioning
//   // epaper.print("FreeFont OK");

//   // // Trigger a full refresh for UC8179:
//   // epaper.update();   // (on TFT_eSPI’s EPaper; use your class’ full-refresh call)

// }

// void loop() {}

// #include "ui/app.h"

// Pull implementations into the sketch TU so the Arduino builder compiles them.
#include "ui/app.cpp"
#include "ui/layout.cpp"
#include "device/display.cpp"
#include "device/wifi.cpp"
#include "transport/http_client.cpp"
#include "transport/json_parse.cpp"
#include "data/metrics_cache.cpp"

// NOTE: Do NOT include charts/barchart.cpp here if it’s already a top-level .cpp compiled by the IDE.
// If it's inside a folder and not getting compiled, include it once here instead.

App myApp;

void setup(){ myApp.setup(); }
void loop() { myApp.loop();  }
