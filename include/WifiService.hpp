#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "AppConfig.hpp"

class WifiService
{
public:
  explicit WifiService(const AppConfig &config);

  void begin();
  void tick();

  bool connected() const;
  String ipAddress() const;
  int32_t rssi() const;
  String hostname() const;
  String ssid() const;

private:
  enum class State
  {
    Disconnected,
    Connecting,
    Connected,
    Backoff,
    NoCredentials
  };

  void beginConnectionAttempt();
  void configureStaticNetworkIfEnabled();
  bool parseIpString(const String &value, IPAddress &out) const;

private:
  const AppConfig &_config;
  State _state;
  uint32_t _lastAttemptAtMs;
  uint32_t _connectStartedAtMs;
  uint32_t _retryDelayMs;

  static constexpr uint32_t kConnectTimeoutMs = 12000;
  static constexpr uint32_t kInitialRetryDelayMs = 1000;
  static constexpr uint32_t kMaxRetryDelayMs = 30000;
};
