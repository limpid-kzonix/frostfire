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

  _server.on("/", HTTP_GET, [this]()
             { handleRoot(); });
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

void ApiServer::handleRoot()
{
  static const char page[] = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Frostfire</title>
  <style>
    :root {
      --bg: radial-gradient(130% 140% at 15% 10%, #0f172a 0%, #020617 45%, #0b1022 100%);
      --panel: rgba(255, 255, 255, 0.06);
      --panel-border: rgba(255, 255, 255, 0.2);
      --text: #e5f2ff;
      --accent: #5eead4;
      --accent-2: #60a5fa;
      --danger: #fb7185;
    }

    * { box-sizing: border-box; }
    html, body { margin: 0; font-family: "Trebuchet MS", "Fira Sans", "Segoe UI", Arial, sans-serif; background: var(--bg); color: var(--text); min-height: 100%; }
    body { display: flex; justify-content: center; align-items: center; padding: 24px; }
    .card {
      width: min(920px, 100%);
      background: linear-gradient(140deg, var(--panel), rgba(8, 16, 36, 0.75));
      border: 1px solid var(--panel-border);
      border-radius: 16px;
      padding: 20px;
      box-shadow: 0 20px 50px rgba(0, 0, 0, 0.45);
      backdrop-filter: blur(4px);
    }
    h1 { margin-top: 0; font-size: 1.7rem; letter-spacing: 0.02em; }
    p { color: #bdd4f4; }
    .row { display: flex; gap: 12px; flex-wrap: wrap; align-items: flex-end; }
    label { display: block; margin-bottom: 6px; color: #b7d0f9; font-size: 0.95rem; }
    input {
      width: 120px;
      border-radius: 8px;
      border: 1px solid #273a66;
      background: rgba(4, 12, 26, 0.8);
      color: #eef4ff;
      padding: 10px;
    }
    input#tokenInput { width: 300px; }
    button {
      border: 0;
      border-radius: 10px;
      padding: 10px 14px;
      color: #021025;
      background: var(--accent);
      font-weight: 700;
      cursor: pointer;
      transition: transform 0.12s ease, opacity 0.2s ease;
    }
    button:hover { transform: translateY(-1px); opacity: 0.95; }
    button:disabled { opacity: 0.5; cursor: not-allowed; }
    #message { margin-top: 12px; min-height: 24px; color: var(--accent-2); }
    .status { margin-top: 16px; display: grid; grid-template-columns: repeat(auto-fit, minmax(190px, 1fr)); gap: 12px; }
    .panel {
      background: rgba(2, 13, 33, 0.6);
      border: 1px solid rgba(96, 165, 250, 0.28);
      border-radius: 10px;
      padding: 12px;
    }
    .label { color: #b9caee; font-size: 0.9rem; margin-bottom: 6px; }
    .value { font-size: 1.2rem; color: #e7f1ff; }
    .warn { color: var(--danger); }
  </style>
</head>
<body>
  <div class="card">
    <h1>Frostfire Relay Control</h1>
    <p>Pulse control is bounded and defaults to a short relay window.</p>
    <div class="row">
      <div>
        <label for="tokenInput">Bearer token</label>
        <input id="tokenInput" placeholder="Set token for control actions" />
      </div>
      <div>
        <label for="durationInput">Duration (ms)</label>
        <input id="durationInput" type="number" min="100" max="3000" step="50" value="500" />
      </div>
      <button id="pulseBtn">Pulse PC Power Button</button>
    </div>
    <div id="message"></div>
    <div class="status">
      <div class="panel">
        <div class="label">Device</div>
        <div id="deviceName" class="value">--</div>
      </div>
      <div class="panel">
        <div class="label">Wi-Fi</div>
        <div id="wifiIp" class="value">--</div>
      </div>
      <div class="panel">
        <div class="label">Relay</div>
        <div id="relayState" class="value">--</div>
      </div>
      <div class="panel">
        <div class="label">Firmware</div>
        <div id="fwVersion" class="value">--</div>
      </div>
      <div class="panel">
        <div class="label">Limits</div>
        <div id="limits" class="value">--</div>
      </div>
    </div>
  </div>
  <script>
    const messageEl = document.getElementById('message');
    const tokenInput = document.getElementById('tokenInput');
    const durationInput = document.getElementById('durationInput');
    const pulseBtn = document.getElementById('pulseBtn');
    const relayState = document.getElementById('relayState');
    const deviceName = document.getElementById('deviceName');
    const wifiIp = document.getElementById('wifiIp');
    const fwVersion = document.getElementById('fwVersion');
    const limits = document.getElementById('limits');

    const toText = (value) => (value === undefined || value === null || value === '' ? '--' : value);
    const parseNumber = (value, fallback) => {
      const parsed = Number(value);
      return Number.isFinite(parsed) ? parsed : fallback;
    };

    const authHeaders = () => {
      const token = tokenInput.value.trim();
      const headers = { 'Content-Type': 'application/json' };
      if (!token) {
        return null;
      }
      headers.Authorization = `Bearer ${token}`;
      return headers;
    };

    async function refreshState() {
      const token = tokenInput.value.trim();
      const headers = token ? { 'Authorization': `Bearer ${token}` } : null;

      try {
        const statusPromise = headers
          ? fetch('/api/v1/status', { headers })
          : fetch('/api/v1/status');

        const [statusResp, healthResp] = await Promise.all([
          statusPromise,
          fetch('/api/v1/health'),
        ]);

        if (!statusResp.ok || !healthResp.ok) {
          if (statusResp.status === 401) {
            messageEl.textContent = 'Status read requires token. Enter token to enable status/pulse.';
          } else {
            messageEl.textContent = 'Unable to read device status.';
          }
          messageEl.className = 'warn';
          return;
        }

        const status = await statusResp.json();
        const health = await healthResp.json();

        deviceName.textContent = toText(status.deviceName);
        relayState.textContent = status.relay && status.relay.active ? 'BUSY' : 'IDLE';
        wifiIp.textContent = `${health.wifi && health.wifi.connected ? 'online' : 'offline'} (${toText(health.wifi ? health.wifi.ip : '0.0.0.0')})`;
        fwVersion.textContent = `${toText(health.device)} / ${toText(health.version)}`;

        const minPulse = toText(status.limits ? status.limits.minPulseMs : '--');
        const maxPulse = toText(status.limits ? status.limits.maxPulseMs : '--');
        const defaultPulse = toText(status.limits ? status.limits.defaultPulseMs : '--');
        limits.textContent = `${minPulse} - ${maxPulse}ms (default ${defaultPulse}ms)`;

        const relayBusy = status.relay && status.relay.active;
        pulseBtn.disabled = relayBusy;
        pulseBtn.textContent = relayBusy ? 'Relay Busy' : 'Pulse PC Power Button';
      } catch (error) {
        messageEl.textContent = `Status refresh failed: ${error.message}`;
        messageEl.className = 'warn';
      }
    }

    async function triggerPulse() {
      const headers = authHeaders();
      if (!headers) {
        messageEl.textContent = 'Provide a token for pulse action.';
        messageEl.className = 'warn';
        return;
      }

      const durationMs = Math.max(100, parseNumber(durationInput.value, 500));
      const payload = { durationMs };
      try {
        const resp = await fetch('/api/v1/power/pulse', {
          method: 'POST',
          headers,
          body: JSON.stringify(payload),
        });
        const data = await resp.json();
        if (!resp.ok) {
          const err = data && data.error ? data.error.message : `request failed (${resp.status})`;
          messageEl.textContent = err;
          messageEl.className = 'warn';
          return;
        }
        messageEl.textContent = `Pulse accepted for ${toText(data.durationMs)}ms.`;
        messageEl.className = '';
      } catch (error) {
        messageEl.textContent = `Pulse request failed: ${error.message}`;
        messageEl.className = 'warn';
      }
      finally {
        refreshState();
      }
    }

    pulseBtn.addEventListener('click', triggerPulse);
    refreshState();
    setInterval(refreshState, 3000);
  </script>
</body>
</html>
)rawliteral";

  _server.send(200, "text/html", page);
}

void ApiServer::handleStatus()
{
  if (!requireAuth("status"))
  {
    return;
  }

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
  if (!requireAuth("config read"))
  {
    return;
  }

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

  if (!_config.save())
  {
    sendError(500, "internal_error", "failed to persist config");
    logger.error("config save failed");
    return;
  }

  logger.info("config updated");
  logger.info(String("  config: deviceName=") + _config.deviceName() +
              ", relayActiveLow=" + String(_config.relayActiveLow() ? "true" : "false") +
              ", defaultPulseMs=" + String(_config.defaultPulseMs()) +
              ", minPulseMs=" + String(_config.minPulseMs()) +
              ", maxPulseMs=" + String(_config.maxPulseMs()));
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
  String body = _server.arg("plain");
  body.trim();
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
