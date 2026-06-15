#include <WiFi.h>

#include "Logger.hpp"
#include "WifiService.hpp"

WifiService::WifiService(const AppConfig &config)
    : _config(config),
      _state(State::Disconnected),
      _lastAttemptAtMs(0),
      _connectStartedAtMs(0),
      _retryDelayMs(kInitialRetryDelayMs)
{
}

void WifiService::begin()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.setHostname(_config.deviceName().c_str());

  if (_config.wifiSsid().isEmpty())
  {
    _state = State::NoCredentials;
    logger.error("wifi credentials missing, cannot start auto-connect");
    return;
  }

  configureStaticNetworkIfEnabled();
  beginConnectionAttempt();
}

void WifiService::tick()
{
  const uint32_t now = millis();

  if (_state == State::Connected)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      _state = State::Disconnected;
      _retryDelayMs = kInitialRetryDelayMs;
      _lastAttemptAtMs = now;
      logger.warn("wifi disconnected");
    }
    return;
  }

  if (_state == State::NoCredentials)
  {
    return;
  }

  if (_state == State::Disconnected)
  {
    if (now - _lastAttemptAtMs >= _retryDelayMs)
    {
      beginConnectionAttempt();
    }
    return;
  }

  if (_state == State::Backoff)
  {
    if (now - _lastAttemptAtMs >= _retryDelayMs)
    {
      beginConnectionAttempt();
    }
    return;
  }

  if (_state == State::Connecting)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      _state = State::Connected;
      logger.info("wifi connected, ip=" + WiFi.localIP().toString());
      return;
    }

    if (now - _connectStartedAtMs >= kConnectTimeoutMs)
    {
      WiFi.disconnect();
      _state = State::Backoff;
      _lastAttemptAtMs = now;
      logger.warn("wifi connect timeout, retrying in " + String(_retryDelayMs) + "ms");
      _retryDelayMs = min(_retryDelayMs * 2, kMaxRetryDelayMs);
    }
    return;
  }
}

bool WifiService::connected() const
{
  return WiFi.status() == WL_CONNECTED;
}

String WifiService::ipAddress() const
{
  if (!connected())
  {
    return "0.0.0.0";
  }
  return WiFi.localIP().toString();
}

int32_t WifiService::rssi() const
{
  if (!connected())
  {
    return 0;
  }
  return WiFi.RSSI();
}

String WifiService::hostname() const
{
  return WiFi.getHostname();
}

String WifiService::ssid() const
{
  if (!connected())
  {
    return "";
  }
  return WiFi.SSID();
}

void WifiService::configureStaticNetworkIfEnabled()
{
  if (!_config.staticIpEnabled())
  {
    return;
  }

  IPAddress staticIp;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;
  if (!parseIpString(_config.staticIp(), staticIp) ||
      !parseIpString(_config.gateway(), gateway) ||
      !parseIpString(_config.subnet(), subnet))
  {
    logger.warn("invalid static network config, falling back to DHCP");
    return;
  }

  if (_config.dns().isEmpty())
  {
    WiFi.config(staticIp, gateway, subnet);
  }
  else if (parseIpString(_config.dns(), dns))
  {
    WiFi.config(staticIp, gateway, subnet, dns);
  }
  else
  {
    logger.warn("invalid DNS in config, ignoring DNS override");
    WiFi.config(staticIp, gateway, subnet);
  }
}

bool WifiService::parseIpString(const String &value, IPAddress &out) const
{
  if (value.isEmpty())
  {
    return false;
  }
  return out.fromString(value);
}

void WifiService::beginConnectionAttempt()
{
  logger.info("wifi connect attempt for " + _config.wifiSsid());
  _connectStartedAtMs = millis();
  _state = State::Connecting;
  WiFi.begin(_config.wifiSsid().c_str(), _config.wifiPassword().c_str());
}

