#pragma once
#include <Arduino.h>
#include <vector>

// Minimal model we need for the CPU usage bar chart
struct MetricsLite {
  float    cpu_pct = 0.0f;
  uint32_t ts      = 0;    // epoch from the Pi
};

struct Cpu { float tempC=0, load1=0, load5=0, load15=0, usagePct=0; };
struct Mem { uint32_t totalMB=0, usedMB=0, freeMB=0; float pct=0; };
struct Fs  { String device, mount, fstype; float totalGB=0, usedGB=0, availGB=0, pct=0; };
struct Iface { String name; String gw; uint16_t mtu=0; bool up=false; String v4; String v6; int speed=0; };
struct Service { String label; bool exists=false, enabled=false, active=false; };
struct SmartDev { String dev; int8_t healthy=-1; }; // -1=unknown, 0=bad, 1=good

struct Metrics {
  String host, os, kernel, timeIso;
  uint32_t uptimeS=0;
  Cpu cpu; Mem mem;
  std::vector<Fs> fs;
  std::vector<Iface> ifaces;
  std::vector<Service> services;
  std::vector<SmartDev> smart;
  bool updatesAvail=false; uint16_t updatesCount=0;
};
