#pragma once
#include <Arduino.h>
#include "globals.h"
#include "driver.h"     // your EPaper wrapper

// Global display instance owned here.
extern EPaper epaper;

// Init + any global display setup
void displayInit();
