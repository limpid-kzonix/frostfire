#include <ArduinoJson.h>

#include "ConfigPolicy.hpp"
#include "PulsePolicy.hpp"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#include "AppConfig.hpp"

AppConfig::AppConfig()
{
  loadDefaults();
}

void AppConfig::loadDefaults()
{
  _deviceName = ConfigPolicy::kDefaultDeviceName;
  _relayPin = ConfigPolicy::kDefaultRelayPin;
  _relayActiveLow = ConfigPolicy::kDefaultRelayActiveLow;
  _defaultPulseMs = ConfigPolicy::kDefaultPulseMs;
  _minPulseMs = ConfigPolicy::kDefaultMinPulseMs;
  _maxPulseMs = ConfigPolicy::kDefaultMaxPulseMs;
  _apiEnabled = ConfigPolicy::kDefaultApiEnabled;
  _otaEnabled = ConfigPolicy::kDefaultOtaEnabled;
  _wifiSsid = defaultWifiSsid();
  _wifiPassword = defaultWifiPassword();
  _staticIpEnabled = ConfigPolicy::kDefaultStaticIpEnabled;
  _staticIp = "";
  _gateway = "";
  _subnet = "";
  _dns = "";
}

String AppConfig::defaultWifiSsid() const
{
#ifdef FROSTFIRE_WIFI_SSID
  return FROSTFIRE_WIFI_SSID;
#else
  return "";
#endif
}

String AppConfig::defaultWifiPassword() const
{
#ifdef FROSTFIRE_WIFI_PASSWORD
  return FROSTFIRE_WIFI_PASSWORD;
#else
  return "";
#endif
}

bool AppConfig::load()
{
  loadDefaults();

  if (!_preferences.begin(kPrefsNamespace, false))
  {
    return false;
  }

  _deviceName = _preferences.getString("deviceName", _deviceName);
  _relayPin = static_cast<uint8_t>(_preferences.getUChar("relayPin", _relayPin));
  _relayActiveLow = _preferences.getBool("relayActiveLow", _relayActiveLow);
  _defaultPulseMs = _preferences.getULong("defaultPulseMs", _defaultPulseMs);
  _minPulseMs = _preferences.getULong("minPulseMs", _minPulseMs);
  _maxPulseMs = _preferences.getULong("maxPulseMs", _maxPulseMs);
  _apiEnabled = _preferences.getBool("apiEnabled", _apiEnabled);
  _otaEnabled = _preferences.getBool("otaEnabled", _otaEnabled);
  _wifiSsid = _preferences.getString("wifiSsid", _wifiSsid);
  _wifiPassword = _preferences.getString("wifiPassword", _wifiPassword);
  _staticIpEnabled = _preferences.getBool("staticIpEnabled", _staticIpEnabled);
  _staticIp = _preferences.getString("staticIp", _staticIp);
  _gateway = _preferences.getString("gateway", _gateway);
  _subnet = _preferences.getString("subnet", _subnet);
  _dns = _preferences.getString("dns", _dns);
  _preferences.end();

  if (!ConfigPolicy::isValidRelayPin(_relayPin))
  {
    _relayPin = ConfigPolicy::kDefaultRelayPin;
  }

  if (_wifiPassword.isEmpty())
  {
    _wifiPassword = defaultWifiPassword();
  }
  if (_wifiSsid.isEmpty())
  {
    _wifiSsid = defaultWifiSsid();
  }

  String pulseError;
  if (!validatePulseConfig(_defaultPulseMs, _minPulseMs, _maxPulseMs, pulseError))
  {
    loadDefaults();
    return false;
  }

  return true;
}

bool AppConfig::save()
{
  if (!_preferences.begin(kPrefsNamespace, true))
  {
    return false;
  }

  _preferences.putString("deviceName", _deviceName);
  _preferences.putUChar("relayPin", _relayPin);
  _preferences.putBool("relayActiveLow", _relayActiveLow);
  _preferences.putULong("defaultPulseMs", _defaultPulseMs);
  _preferences.putULong("minPulseMs", _minPulseMs);
  _preferences.putULong("maxPulseMs", _maxPulseMs);
  _preferences.putBool("apiEnabled", _apiEnabled);
  _preferences.putBool("otaEnabled", _otaEnabled);
  _preferences.putString("wifiSsid", _wifiSsid);
  _preferences.putString("wifiPassword", _wifiPassword);
  _preferences.putBool("staticIpEnabled", _staticIpEnabled);
  _preferences.putString("staticIp", _staticIp);
  _preferences.putString("gateway", _gateway);
  _preferences.putString("subnet", _subnet);
  _preferences.putString("dns", _dns);

  _preferences.end();
  return true;
}

bool AppConfig::validatePulseConfig(uint32_t defaultPulseMs, uint32_t minPulseMs, uint32_t maxPulseMs, String &errorMessage) const
{
  const auto error = PulsePolicy::validatePulseConfig(defaultPulseMs, minPulseMs, maxPulseMs);
  if (error == PulsePolicy::ValidationError::None)
  {
    return true;
  }

  errorMessage = PulsePolicy::validationErrorMessage(error);
  return false;
}

bool AppConfig::applyRuntimeUpdate(const JsonVariantConst &payload, String &errorMessage, bool &relayActiveLowChanged)
{
  relayActiveLowChanged = false;
  bool touched = false;

  uint32_t defaultPulseMs = _defaultPulseMs;
  uint32_t minPulseMs = _minPulseMs;
  uint32_t maxPulseMs = _maxPulseMs;
  bool relayActiveLow = _relayActiveLow;
  String deviceName = _deviceName;

  if (!payload.is<JsonObject>())
  {
    errorMessage = "payload must be a JSON object";
    return false;
  }

  auto obj = payload.as<JsonObject>();

  if (obj.containsKey("deviceName"))
  {
    if (!obj["deviceName"].is<const char *>())
    {
      errorMessage = "deviceName must be a string";
      return false;
    }
    deviceName = obj["deviceName"].as<String>();
    if (deviceName.isEmpty())
    {
      errorMessage = "deviceName cannot be empty";
      return false;
    }
    touched = true;
  }

  if (obj.containsKey("relayActiveLow"))
  {
    if (!obj["relayActiveLow"].is<bool>())
    {
      errorMessage = "relayActiveLow must be a boolean";
      return false;
    }
    relayActiveLow = obj["relayActiveLow"].as<bool>();
    touched = true;
    if (relayActiveLow != _relayActiveLow)
    {
      relayActiveLowChanged = true;
    }
  }

  if (obj.containsKey("defaultPulseMs"))
  {
    if (!obj["defaultPulseMs"].is<uint32_t>())
    {
      errorMessage = "defaultPulseMs must be a positive integer";
      return false;
    }
    defaultPulseMs = obj["defaultPulseMs"].as<uint32_t>();
    touched = true;
  }

  if (obj.containsKey("minPulseMs"))
  {
    if (!obj["minPulseMs"].is<uint32_t>())
    {
      errorMessage = "minPulseMs must be a positive integer";
      return false;
    }
    minPulseMs = obj["minPulseMs"].as<uint32_t>();
    touched = true;
  }

  if (obj.containsKey("maxPulseMs"))
  {
    if (!obj["maxPulseMs"].is<uint32_t>())
    {
      errorMessage = "maxPulseMs must be a positive integer";
      return false;
    }
    maxPulseMs = obj["maxPulseMs"].as<uint32_t>();
    touched = true;
  }

  if (!validatePulseConfig(defaultPulseMs, minPulseMs, maxPulseMs, errorMessage))
  {
    return false;
  }

  if (!touched)
  {
    errorMessage = "no valid config fields provided";
    return false;
  }

  _deviceName = deviceName;
  _defaultPulseMs = defaultPulseMs;
  _minPulseMs = minPulseMs;
  _maxPulseMs = maxPulseMs;
  _relayActiveLow = relayActiveLow;

  return true;
}

const String &AppConfig::deviceName() const
{
  return _deviceName;
}

uint8_t AppConfig::relayPin() const
{
  return _relayPin;
}

bool AppConfig::relayActiveLow() const
{
  return _relayActiveLow;
}

uint32_t AppConfig::defaultPulseMs() const
{
  return _defaultPulseMs;
}

uint32_t AppConfig::minPulseMs() const
{
  return _minPulseMs;
}

uint32_t AppConfig::maxPulseMs() const
{
  return _maxPulseMs;
}

bool AppConfig::apiEnabled() const
{
  return _apiEnabled;
}

bool AppConfig::otaEnabled() const
{
  return _otaEnabled;
}

const String &AppConfig::wifiSsid() const
{
  return _wifiSsid;
}

const String &AppConfig::wifiPassword() const
{
  return _wifiPassword;
}

bool AppConfig::staticIpEnabled() const
{
  return _staticIpEnabled;
}

const String &AppConfig::staticIp() const
{
  return _staticIp;
}

const String &AppConfig::gateway() const
{
  return _gateway;
}

const String &AppConfig::subnet() const
{
  return _subnet;
}

const String &AppConfig::dns() const
{
  return _dns;
}

void AppConfig::setDeviceName(const String &value)
{
  _deviceName = value;
}

void AppConfig::setRelayPin(uint8_t value)
{
  _relayPin = value;
}

void AppConfig::setRelayActiveLow(bool value)
{
  _relayActiveLow = value;
}

void AppConfig::setDefaultPulseMs(uint32_t value)
{
  _defaultPulseMs = value;
}

void AppConfig::setMinPulseMs(uint32_t value)
{
  _minPulseMs = value;
}

void AppConfig::setMaxPulseMs(uint32_t value)
{
  _maxPulseMs = value;
}

void AppConfig::setApiEnabled(bool value)
{
  _apiEnabled = value;
}

void AppConfig::setOtaEnabled(bool value)
{
  _otaEnabled = value;
}

void AppConfig::setWifiSsid(const String &value)
{
  _wifiSsid = value;
}

void AppConfig::setWifiPassword(const String &value)
{
  _wifiPassword = value;
}

void AppConfig::setStaticIpEnabled(bool value)
{
  _staticIpEnabled = value;
}

void AppConfig::setStaticIp(const String &value)
{
  _staticIp = value;
}

void AppConfig::setGateway(const String &value)
{
  _gateway = value;
}

void AppConfig::setSubnet(const String &value)
{
  _subnet = value;
}

void AppConfig::setDns(const String &value)
{
  _dns = value;
}
