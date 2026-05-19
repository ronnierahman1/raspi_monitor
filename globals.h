
#pragma once
#include <TFT_eSPI.h>   // brings EPaper type + TFT_* colors

// Single shared EPaper instance is defined in Weather.ino
extern EPaper epaper;
#define SCR_W  epaper.width()
#define SCR_H  epaper.height()