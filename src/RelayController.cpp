#include <Arduino.h>

#include "Logger.hpp"
#include "RelayController.hpp"

RelayController::RelayController(const AppConfig &config)
    : _config(config),
      _pin(config.relayPin()),
      _isActive(false),
      _isBusy(false),
      _lastPulseAtMs(0),
      _lastPulseDurationMs(0)
{
}

void RelayController::begin()
{
  _pin = _config.relayPin();
  pinMode(_pin, OUTPUT);

  bool relayActiveLow = _config.relayActiveLow();
  const uint8_t offLevel = relayActiveLow ? HIGH : LOW;

  digitalWrite(_pin, offLevel);
  _isActive = false;
  _isBusy = false;

  logger.info("relay initialized on pin " + String(_pin));
}

void RelayController::tick()
{
  if (!_isBusy)
  {
    return;
  }

  const uint32_t now = millis();
  if (now - _lastPulseAtMs >= _lastPulseDurationMs)
  {
    logger.warn("relay pulse window complete");
    forceOff();
  }
}

RelayResult RelayController::requestPulse(uint32_t durationMs)
{
  if (_isBusy)
  {
    return RelayResult::Busy;
  }

  if (!validateDuration(durationMs))
  {
    return RelayResult::InvalidDuration;
  }

  setRelayLevel(true);
  _lastPulseAtMs = millis();
  _lastPulseDurationMs = durationMs;
  _isBusy = true;
  _isActive = true;
  logger.info("relay pulse accepted");
  return RelayResult::Accepted;
}

bool RelayController::isActive() const
{
  return _isActive;
}

bool RelayController::isBusy() const
{
  return _isBusy;
}

uint32_t RelayController::lastPulseAtMs() const
{
  return _lastPulseAtMs;
}

uint32_t RelayController::lastPulseDurationMs() const
{
  return _lastPulseDurationMs;
}

void RelayController::forceOff()
{
  setRelayLevel(false);
  _isActive = false;
  _isBusy = false;
}

void RelayController::setRelayLevel(bool active)
{
  const bool relayActiveLow = _config.relayActiveLow();
  const uint8_t level = (active == relayActiveLow) ? LOW : HIGH;
  digitalWrite(_pin, level);
}

bool RelayController::validateDuration(uint32_t durationMs) const
{
  return durationMs >= _config.minPulseMs() && durationMs <= _config.maxPulseMs();
}
