#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <Arduino.h>

struct PowerData {
  float busVoltage = 0.0f;
  float shuntVoltage = 0.0f;
  float currentA = 0.0f;
  float powerW = 0.0f;
  bool meterOk = false;

  // Compatibility names used by existing modules.
  float estimatedVoltage = 0.0f;
  float estimatedPowerW = 0.0f;
};

void beginPowerMonitor();
void readPowerMonitor(PowerData &data);

#endif
