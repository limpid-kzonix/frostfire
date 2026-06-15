#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>
#include <Preferences.h>

class AppConfig
{
public:
  AppConfig();

  bool load();
  bool save();
  bool applyRuntimeUpdate(const JsonVariantConst &payload, String &errorMessage, bool &relayActiveLowChanged);

  const String &deviceName() const;
  uint8_t relayPin() const;
  bool relayActiveLow() const;
  uint32_t defaultPulseMs() const;
  uint32_t minPulseMs() const;
  uint32_t maxPulseMs() const;
  bool apiEnabled() const;
  bool otaEnabled() const;

  const String &wifiSsid() const;
  const String &wifiPassword() const;
  bool staticIpEnabled() const;
  const String &staticIp() const;
  const String &gateway() const;
  const String &subnet() const;
  const String &dns() const;

  bool validatePulseConfig(uint32_t defaultPulseMs, uint32_t minPulseMs, uint32_t maxPulseMs, String &errorMessage) const;
  void setDeviceName(const String &value);
  void setRelayPin(uint8_t value);
  void setRelayActiveLow(bool value);
  void setDefaultPulseMs(uint32_t value);
  void setMinPulseMs(uint32_t value);
  void setMaxPulseMs(uint32_t value);
  void setApiEnabled(bool value);
  void setOtaEnabled(bool value);
  void setWifiSsid(const String &value);
  void setWifiPassword(const String &value);
  void setStaticIpEnabled(bool value);
  void setStaticIp(const String &value);
  void setGateway(const String &value);
  void setSubnet(const String &value);
  void setDns(const String &value);

private:
  void loadDefaults();

  String defaultWifiSsid() const;
  String defaultWifiPassword() const;

private:
  static constexpr const char *kPrefsNamespace = "frostfire_cfg";

  Preferences _preferences;

  String _deviceName;
  uint8_t _relayPin;
  bool _relayActiveLow;
  uint32_t _defaultPulseMs;
  uint32_t _minPulseMs;
  uint32_t _maxPulseMs;
  bool _apiEnabled;
  bool _otaEnabled;

  String _wifiSsid;
  String _wifiPassword;
  bool _staticIpEnabled;
  String _staticIp;
  String _gateway;
  String _subnet;
  String _dns;
};
