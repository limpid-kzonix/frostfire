#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>

#include "AppConfig.hpp"
#include "Auth.hpp"
#include "RelayController.hpp"
#include "WifiService.hpp"

class ApiServer
{
public:
  ApiServer(AppConfig &config, RelayController &relay, WifiService &wifi, const String &authToken);

  void begin();
  void tick();

private:
  void handleHealth();
  void handleStatus();
  void handlePowerPulse();
  void handleRelayPulse();
  void handleConfigGet();
  void handleConfigPost();
  void handleReboot();
  bool parseDurationFromBody(uint32_t &durationMs, String &errorCode, String &errorMessage);
  void sendNotFound();
  void sendJson(int statusCode, const JsonDocument &document);
  void sendError(int statusCode, const String &errorCode, const String &errorMessage);
  bool requireAuth(const char *operation);

private:
  static constexpr uint16_t kPort = 80;
  static constexpr uint16_t kRebootDelayMs = 120;

  AppConfig &_config;
  RelayController &_relay;
  WifiService &_wifi;
  WebServer _server;
  Auth _auth;
  uint32_t _rebootAtMs;
  bool _rebootScheduled;
};
