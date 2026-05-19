#include "device/display.h"

EPaper epaper;  // single definition

void displayInit() {
  epaper.init();
  epaper.setRotation(0);
  // If your panel boots ghosted, you can do a double clear+update once here.
  // epaper.fillRect(0,0,SCR_W,SCR_H,TFT_WHITE); epaper.update();
  // epaper.fillRect(0,0,SCR_W,SCR_H,TFT_WHITE); epaper.update();
}
