#pragma once

#include <Arduino.h>

#include "AppConfig.hpp"

enum class RelayResult
{
  Accepted,
  Busy,
  InvalidDuration,
  InternalError
};

class RelayController
{
public:
  explicit RelayController(const AppConfig &config);

  void begin();
  void tick();

  RelayResult requestPulse(uint32_t durationMs);

  bool isActive() const;
  bool isBusy() const;
  uint32_t lastPulseAtMs() const;
  uint32_t lastPulseDurationMs() const;
  void forceOff();

private:
  void setRelayLevel(bool active);
  bool validateDuration(uint32_t durationMs) const;

private:
  const AppConfig &_config;
  uint8_t _pin;
  bool _isActive;
  bool _isBusy;
  uint32_t _lastPulseAtMs;
  uint32_t _lastPulseDurationMs;
};
