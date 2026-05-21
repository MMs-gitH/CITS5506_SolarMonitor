#include "PowerMonitor.h"
#include "Config.h"

#include <Wire.h>
#include <math.h>

static const uint8_t INA219_REG_CONFIG = 0x00;
static const uint8_t INA219_REG_SHUNT_VOLTAGE = 0x01;
static const uint8_t INA219_REG_BUS_VOLTAGE = 0x02;
static const uint8_t INA219_REG_CURRENT = 0x04;
static const uint8_t INA219_REG_CALIBRATION = 0x05;
static const uint8_t SUPPORTED_INA219_ADDRESSES[] = {0x40, 0x41, 0x44, 0x45};

static uint8_t activeIna219Address = INA219_ADDRESS;
static float currentLsbA = INA219_MAX_EXPECTED_CURRENT_A / 32768.0f;
static uint16_t calibrationValue = 0;
static float filteredCurrentA = 0.0f;
static bool currentFilterReady = false;
static int currentZeroStreak = 0;

static void resetCurrentFilter() {
  filteredCurrentA = 0.0f;
  currentFilterReady = false;
  currentZeroStreak = 0;
}

static float filterCurrent(float current) {
  if (!currentFilterReady) {
    filteredCurrentA = current;
    currentFilterReady = true;
  } else {
    filteredCurrentA += CURRENT_FILTER_ALPHA * (current - filteredCurrentA);
  }

  return filteredCurrentA;
}

static bool writeRegister(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(activeIna219Address);
  Wire.write(reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  return Wire.endTransmission() == 0;
}

static bool readRegister(uint8_t reg, uint16_t &value) {
  Wire.beginTransmission(activeIna219Address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  int bytesRead = Wire.requestFrom(activeIna219Address, (uint8_t)2);
  if (bytesRead != 2) {
    return false;
  }

  value = ((uint16_t)Wire.read() << 8) | (uint16_t)Wire.read();
  return true;
}

static bool configureIna219() {
  if (INA219_SHUNT_OHMS <= 0.0f || INA219_MAX_EXPECTED_CURRENT_A <= 0.0f) {
    return false;
  }

  currentLsbA = INA219_MAX_EXPECTED_CURRENT_A / 32768.0f;

  float calibration = 0.04096f / (currentLsbA * INA219_SHUNT_OHMS);
  if (calibration < 1.0f) {
    calibration = 1.0f;
  }
  if (calibration > 65535.0f) {
    calibration = 65535.0f;
  }
  calibrationValue = (uint16_t)calibration;

  // 32 V bus range, +/-320 mV shunt range, 12-bit ADC, shunt+bus continuous.
  const uint16_t config = 0x399F;
  return writeRegister(INA219_REG_CALIBRATION, calibrationValue)
    && writeRegister(INA219_REG_CONFIG, config);
}

void beginPowerMonitor() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  activeIna219Address = INA219_ADDRESS;
  bool configured = configureIna219();
  if (!configured) {
    for (uint8_t address : SUPPORTED_INA219_ADDRESSES) {
      activeIna219Address = address;
      configured = configureIna219();
      if (configured) {
        break;
      }
    }
  }

  if (configured) {
    Serial.print("I2C power meter ready at 0x");
    Serial.println(activeIna219Address, HEX);
  } else {
    Serial.println("I2C power meter not found. Check SDA/SCL, power, and DIP address.");
  }
}

void readPowerMonitor(PowerData &data) {
  uint16_t rawBus = 0;
  uint16_t rawShunt = 0;
  uint16_t rawCurrent = 0;

  // Keep calibration fresh in case the INA219 has been reset.
  writeRegister(INA219_REG_CALIBRATION, calibrationValue);

  bool ok = readRegister(INA219_REG_BUS_VOLTAGE, rawBus)
    && readRegister(INA219_REG_SHUNT_VOLTAGE, rawShunt)
    && readRegister(INA219_REG_CURRENT, rawCurrent);

  if (!ok) {
    data.meterOk = false;
    data.busVoltage = 0.0f;
    data.shuntVoltage = 0.0f;
    data.currentA = 0.0f;
    data.powerW = 0.0f;
    data.estimatedVoltage = 0.0f;
    data.estimatedPowerW = 0.0f;
    resetCurrentFilter();
    return;
  }

  float busVoltage = (float)(rawBus >> 3) * 0.004f * POWER_VOLTAGE_CALIBRATION;
  float shuntVoltage = (float)((int16_t)rawShunt) * 0.00001f;
  float current = (float)((int16_t)rawCurrent) * currentLsbA * POWER_CURRENT_DIRECTION * POWER_CURRENT_CALIBRATION;

  if (!isfinite(busVoltage) || !isfinite(shuntVoltage) || !isfinite(current)) {
    data.meterOk = false;
    resetCurrentFilter();
    return;
  }

  if (current < CURRENT_NOISE_FLOOR_A) {
    current = 0.0f;
  }

  if (current <= 0.0f) {
    currentZeroStreak++;
    if (currentZeroStreak >= CURRENT_ZERO_RESET_COUNT) {
      resetCurrentFilter();
      current = 0.0f;
    } else {
      current = filterCurrent(0.0f);
    }
  } else {
    currentZeroStreak = 0;
    current = filterCurrent(current);
  }

  if (busVoltage < 0.0f) {
    busVoltage = 0.0f;
  }
  if (current < 0.0f) {
    current = 0.0f;
  }

  // Use rated voltage as fallback when bus voltage reads near zero
  // (e.g. panel in shade or VIN+ wiring issue).
  float effectiveVoltage = (busVoltage >= PANEL_VOLTAGE_MIN_V) ? busVoltage : PANEL_RATED_VOLTAGE_V;

  data.meterOk = true;
  data.busVoltage = busVoltage;
  data.shuntVoltage = shuntVoltage;
  data.currentA = current;
  data.powerW = effectiveVoltage * current;
  data.estimatedVoltage = effectiveVoltage;
  data.estimatedPowerW = data.powerW;
}
