#pragma once

#include <Arduino.h>

#include "AppConfig.hpp"
#include "RelayController.hpp"

class OtaService
{
public:
  OtaService(const AppConfig &config, const RelayController &relay);

  void begin();
  void tick();

private:
#ifdef FROSTFIRE_OTA_ENABLED
  void configureCallbacks();
#endif

private:
  const AppConfig &_config;
  const RelayController &_relay;
  const String _password;
  bool _initialized;
  bool _pausedByRelay;
};
