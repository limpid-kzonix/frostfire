#include <Arduino.h>

#include "ApiServer.hpp"
#include "BuildInfo.hpp"
#include "Logger.hpp"

ApiServer::ApiServer(AppConfig &config, RelayController &relay, WifiService &wifi, const String &authToken)
    : _config(config), _relay(relay), _wifi(wifi), _server(kPort), _auth(authToken), _rebootAtMs(0), _rebootScheduled(false)
{
}

void ApiServer::begin()
{
  if (!_config.apiEnabled())
  {
    logger.warn("api disabled via config");
    return;
  }

  _server.on("/api/v1/health", HTTP_GET, [this]()
             { handleHealth(); });
  _server.on("/api/v1/status", HTTP_GET, [this]()
             { handleStatus(); });
  _server.on("/api/v1/power/pulse", HTTP_POST, [this]()
             { handlePowerPulse(); });
  _server.on("/api/v1/relay/pulse", HTTP_POST, [this]()
             { handleRelayPulse(); });
  _server.on("/api/v1/config", HTTP_GET, [this]()
             { handleConfigGet(); });
  _server.on("/api/v1/config", HTTP_POST, [this]()
             { handleConfigPost(); });
  _server.on("/api/v1/reboot", HTTP_POST, [this]()
             { handleReboot(); });
  _server.onNotFound([this]()
                    { sendNotFound(); });
  _server.begin();
  logger.info("api server started on port " + String(kPort));
}

void ApiServer::tick()
{
  _server.handleClient();
  if (_rebootScheduled && millis() - _rebootAtMs >= kRebootDelayMs)
  {
    logger.warn("rebooting now");
    delay(20);
    ESP.restart();
  }
}

void ApiServer::sendJson(int statusCode, const JsonDocument &document)
{
  String payload;
  serializeJson(document, payload);
  _server.send(statusCode, "application/json", payload);
}

void ApiServer::sendError(int statusCode, const String &errorCode, const String &errorMessage)
{
  StaticJsonDocument<192> doc;
  JsonObject error = doc.createNestedObject("error");
  error["code"] = errorCode;
  error["message"] = errorMessage;
  sendJson(statusCode, doc);
}

bool ApiServer::requireAuth(const char *operation)
{
  if (!_config.apiEnabled())
  {
    return false;
  }
  if (!_auth.isAuthorized(_server.header("Authorization")))
  {
    sendError(401, "unauthorized", String("unauthorized access for ") + operation);
    return false;
  }
  return true;
}

void ApiServer::handleHealth()
{
  StaticJsonDocument<256> doc;
  JsonObject wifi = doc.createNestedObject("wifi");
  JsonObject relay = doc.createNestedObject("relay");

  doc["ok"] = true;
  doc["device"] = _config.deviceName();
  doc["version"] = getFirmwareVersion();
  doc["uptimeMs"] = millis();
  wifi["connected"] = _wifi.connected();
  wifi["ip"] = _wifi.ipAddress();
  wifi["rssi"] = _wifi.rssi();
  relay["active"] = _relay.isActive();
  doc["heapFree"] = ESP.getFreeHeap();

  sendJson(200, doc);
}

void ApiServer::handleStatus()
{
  StaticJsonDocument<384> doc;
  JsonObject relay = doc.createNestedObject("relay");
  JsonObject limits = doc.createNestedObject("limits");

  doc["deviceName"] = _config.deviceName();
  relay["pin"] = _config.relayPin();
  relay["activeLow"] = _config.relayActiveLow();
  relay["active"] = _relay.isActive();
  relay["lastPulseAtMs"] = _relay.lastPulseAtMs();
  relay["lastPulseDurationMs"] = _relay.lastPulseDurationMs();
  limits["minPulseMs"] = _config.minPulseMs();
  limits["maxPulseMs"] = _config.maxPulseMs();
  limits["defaultPulseMs"] = _config.defaultPulseMs();

  sendJson(200, doc);
}

void ApiServer::handlePowerPulse()
{
  if (!requireAuth("power pulse"))
  {
    return;
  }

  uint32_t durationMs;
  String errorCode;
  String errorMessage;
  if (!parseDurationFromBody(durationMs, errorCode, errorMessage))
  {
    sendError(400, errorCode, errorMessage);
    return;
  }

  const RelayResult result = _relay.requestPulse(durationMs);
  if (result == RelayResult::Busy)
  {
    sendError(409, "relay_busy", "relay already pulsing");
    return;
  }
  if (result == RelayResult::InvalidDuration)
  {
    sendError(400, "invalid_duration", "durationMs must be within configured bounds");
    return;
  }
  if (result == RelayResult::InternalError)
  {
    sendError(500, "internal_error", "relay request failed");
    return;
  }

  StaticJsonDocument<96> doc;
  doc["accepted"] = true;
  doc["durationMs"] = durationMs;
  sendJson(202, doc);
}

void ApiServer::handleRelayPulse()
{
  handlePowerPulse();
}

void ApiServer::handleConfigGet()
{
  StaticJsonDocument<256> doc;
  JsonObject relay = doc.createNestedObject("relay");
  JsonObject limits = doc.createNestedObject("limits");
  JsonObject wifi = doc.createNestedObject("wifi");
  JsonObject feature = doc.createNestedObject("feature");

  doc["deviceName"] = _config.deviceName();
  relay["pin"] = _config.relayPin();
  relay["activeLow"] = _config.relayActiveLow();
  limits["defaultPulseMs"] = _config.defaultPulseMs();
  limits["minPulseMs"] = _config.minPulseMs();
  limits["maxPulseMs"] = _config.maxPulseMs();
  wifi["ssid"] = _config.wifiSsid();
  wifi["staticIpEnabled"] = _config.staticIpEnabled();
  wifi["staticIp"] = _config.staticIp();
  wifi["gateway"] = _config.gateway();
  wifi["subnet"] = _config.subnet();
  wifi["dns"] = _config.dns();
  feature["apiEnabled"] = _config.apiEnabled();
  feature["otaEnabled"] = _config.otaEnabled();
  feature["version"] = getFirmwareVersion();
  feature["device"] = getFirmwareName();

  sendJson(200, doc);
}

void ApiServer::handleConfigPost()
{
  if (!requireAuth("config update"))
  {
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError decodeError = deserializeJson(doc, _server.arg("plain"));
  if (decodeError)
  {
    sendError(400, "invalid_json", "request body must be valid JSON");
    return;
  }

  String errorMessage;
  bool relayActiveLowChanged = false;
  if (!_config.applyRuntimeUpdate(doc.as<JsonVariantConst>(), errorMessage, relayActiveLowChanged))
  {
    sendError(400, "invalid_config", errorMessage);
    return;
  }

  _config.save();
  logger.info("config updated");
  if (relayActiveLowChanged)
  {
    _relay.begin();
  }

  StaticJsonDocument<128> response;
  response["updated"] = true;
  response["relayActiveLow"] = _config.relayActiveLow();
  sendJson(200, response);
}

void ApiServer::handleReboot()
{
  if (!requireAuth("reboot"))
  {
    return;
  }

  StaticJsonDocument<96> doc;
  doc["accepted"] = true;
  sendJson(202, doc);
  _rebootScheduled = true;
  _rebootAtMs = millis();
  logger.warn("reboot requested");
}

void ApiServer::sendNotFound()
{
  sendError(404, "not_found", "endpoint not found");
}

bool ApiServer::parseDurationFromBody(uint32_t &durationMs, String &errorCode, String &errorMessage)
{
  const String body = _server.arg("plain");
  if (body.isEmpty())
  {
    durationMs = _config.defaultPulseMs();
    return true;
  }

  StaticJsonDocument<128> payload;
  DeserializationError decodeError = deserializeJson(payload, body);
  if (decodeError)
  {
    errorCode = "invalid_json";
    errorMessage = "request body must be valid JSON";
    return false;
  }

  if (!payload.is<JsonObject>())
  {
    errorCode = "invalid_json";
    errorMessage = "payload must be a JSON object";
    return false;
  }

  if (!payload.containsKey("durationMs"))
  {
    durationMs = _config.defaultPulseMs();
    return true;
  }

  if (!payload["durationMs"].is<uint32_t>())
  {
    errorCode = "invalid_duration";
    errorMessage = "durationMs must be a positive integer";
    return false;
  }

  durationMs = payload["durationMs"].as<uint32_t>();
  return true;
}
