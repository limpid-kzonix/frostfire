
# Frostfire ESP32 Firmware Implementation Plan for Coding Agent

You are working on an existing embedded project named `Frostfire`.

Project goal:
Implement production-quality ESP32 firmware using PlatformIO and the Arduino framework for remotely controlling a PC power switch through a relay module.

Important language note:
Although the user says “C”, PlatformIO with Arduino framework for ESP32 is practically C++ firmware. Keep the code simple, C-like, deterministic, and embedded-friendly, but use `.cpp/.hpp` where required by Arduino/PlatformIO conventions.

Primary function:
The ESP32 must expose a local network control interface that can safely trigger a relay connected to the PC motherboard power-button pins. The relay must simulate a human pressing the PC power button for a short configurable pulse, not hold the relay indefinitely by default.

Strict safety rule:
Never design the default command as an uncontrolled toggle. The default operation must be a bounded pulse: relay ON for N milliseconds, then relay OFF, even if HTTP client disconnects, Wi-Fi drops, or an internal error occurs.

Target hardware:
ESP32 development board.
Single-channel relay module.
Relay input pin connected to an ESP32 GPIO.
Relay contact connected across the PC motherboard power-button header.
ESP32 powered via USB or stable 5V supply.

Electrical assumptions:
Relay module may be active LOW or active HIGH. This must be configurable.
Default GPIO should be configurable, with a safe initial state applied before pin mode changes where possible.
Relay must be OFF during boot, reset, Wi-Fi reconnect, and firmware initialization.
Avoid ESP32 strapping pins unless explicitly configured by the user.
Recommended default GPIO: 5, 18, 19, 23, 25, 26, or 27 depending on board layout. Avoid GPIO 0, 2, 12, 15 for default relay control unless the user confirms board-specific behavior.

Implementation stack:
Use PlatformIO.
Use Arduino framework for ESP32.
Use built-in ESP32 Arduino Wi-Fi primitives.
Use `Preferences` for persistent small configuration.
Use ArduinoJson v7 for JSON request/response bodies.
Use ESPAsyncWebServer from the community-maintained `ESP32Async` organization if async HTTP is selected.
Alternatively, use built-in synchronous `WebServer` for minimal firmware. Prefer async only if the existing project already uses it or if multiple concurrent clients/UI files are needed.
Use WiFiManager only if the existing project lacks Wi-Fi provisioning and the user wants captive-portal setup. Otherwise preserve existing static configuration model.
Use ArduinoOTA optionally behind a compile-time flag. Do not enable OTA by default without authentication.

Expected PlatformIO structure:

```text
Frostfire/
  platformio.ini
  README.md
  LICENSE
  include/
    AppConfig.hpp
    BuildInfo.hpp
    RelayController.hpp
    WifiService.hpp
    ApiServer.hpp
    Auth.hpp
    Health.hpp
    Logger.hpp
  src/
    main.cpp
    AppConfig.cpp
    RelayController.cpp
    WifiService.cpp
    ApiServer.cpp
    Auth.cpp
    Health.cpp
    Logger.cpp
  data/
    index.html              optional minimal UI
  test/
    test_relay_controller/
    test_config/
  docs/
    wiring.md
    api.md
    safety.md
```

Recommended `platformio.ini` baseline:

```ini
[platformio]
default_envs = esp32dev

[env:esp32dev]
platform = platformio/espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

build_flags =
  -D FROSTFIRE_FW_NAME=\"Frostfire\"
  -D FROSTFIRE_FW_VERSION=\"0.1.0\"
  -D CORE_DEBUG_LEVEL=3

lib_deps =
  bblanchon/ArduinoJson
```

If async HTTP is used, add:

```ini
lib_deps =
  bblanchon/ArduinoJson
  ESP32Async/ESPAsyncWebServer
  ESP32Async/AsyncTCP
```

If WiFiManager is used, add:

```ini
lib_deps =
  bblanchon/ArduinoJson
  tzapu/WiFiManager
```

The coding agent must inspect the existing repository first and avoid rewriting working code blindly. It must preserve existing behavior where safe and explicitly migrate only what improves reliability, structure, and maintainability.

Implementation phases:

Phase 1: Repository audit.
Inspect current files, `platformio.ini`, existing `.ino/.cpp/.h` files, pin definitions, Wi-Fi handling, relay logic, API endpoints, and README.
Identify whether the project is currently Arduino IDE style or already PlatformIO style.
If an `.ino` exists, migrate it into `src/main.cpp`.
Remove hardcoded Wi-Fi credentials from source code.
Check whether relay control is blocking, unsafe, or based on unbounded toggle.
Check whether the relay state is safe during boot.
Check whether any secrets are committed.
Check whether the project has documentation for wiring and usage.

Phase 2: PlatformIO normalization.
Ensure the project builds using `pio run`.
Ensure board is configurable through PlatformIO environment.
Keep one default environment named `esp32dev`.
Add comments in `platformio.ini` explaining how to change board type.
Add `monitor_filters = esp32_exception_decoder` if available and useful.
Do not pin random obsolete library versions unless compatibility requires it.
Prefer minimal dependencies.

Phase 3: Configuration model.
Create `AppConfig`.
Required config fields:
`deviceName`
`relayPin`
`relayActiveLow`
`defaultPulseMs`
`maxPulseMs`
`minPulseMs`
`authTokenHash` or plain token only if explicitly documented as local-only prototype
`wifiSsid`
`wifiPassword`
`staticIpEnabled`
`staticIp`
`gateway`
`subnet`
`dns`
`apiEnabled`
`otaEnabled`

Default values:
`deviceName = "frostfire"`
`relayPin = 5`
`relayActiveLow = true` or detected from existing project
`defaultPulseMs = 500`
`minPulseMs = 100`
`maxPulseMs = 3000`
`apiEnabled = true`
`otaEnabled = false`

Persist small config values using ESP32 `Preferences`.
Do not store large HTML assets in Preferences.
If a web UI is needed, use LittleFS only after confirming project need.

Phase 4: Relay controller.
Create `RelayController`.
Responsibilities:
Initialize relay GPIO safely.
Guarantee relay OFF at startup.
Expose `pulse(durationMs)`.
Expose `on()`, `off()` only as internal/private or protected diagnostic operations.
Expose `isActive()`.
Expose `lastPulseAt`.
Expose `lastPulseDurationMs`.
Reject pulse duration below min or above max.
Reject concurrent pulse requests unless explicitly queued. Prefer rejecting with HTTP 409.
Never leave relay active if an exception-like error path occurs.
Use `millis()`-based non-blocking timing if possible.
If simple blocking pulse is used, keep it isolated and bounded; no infinite waits.

Preferred non-blocking design:
`RelayController::requestPulse(uint32_t durationMs)` stores deadline.
Main loop calls `relayController.tick()`.
When deadline passes, relay is turned OFF.
This avoids blocking Wi-Fi/web server tasks.

Relay state mapping:
If `relayActiveLow == true`:
OFF = HIGH
ON = LOW

If `relayActiveLow == false`:
OFF = LOW
ON = HIGH

Initialization sequence:
Compute OFF level.
Write OFF level before or immediately after `pinMode`.
Set pin mode OUTPUT.
Write OFF again.
Delay only if absolutely needed.

Phase 5: Wi-Fi service.
Create `WifiService`.
Responsibilities:
Connect to Wi-Fi.
Report connection state.
Reconnect with bounded retry/backoff.
Expose IP address, RSSI, hostname.
Avoid tight reconnect loops.
Do not block forever waiting for Wi-Fi.
Set hostname from config.
Support either:
A. compile-time credentials through `secrets.h` ignored by git;
B. runtime provisioning with WiFiManager;
C. serial/env-style config for development.

Preferred for existing project:
If credentials are currently hardcoded, replace them with `include/secrets.example.h` and `.gitignore` entry for `include/secrets.h`.

Example:

```cpp
// include/secrets.example.h
#pragma once

#define FROSTFIRE_WIFI_SSID "your-ssid"
#define FROSTFIRE_WIFI_PASSWORD "your-password"
#define FROSTFIRE_API_TOKEN "change-me"
```

Do not commit real credentials.

Phase 6: HTTP API.
Create `ApiServer`.
Default port: 80.
All mutating endpoints must require authentication unless auth is explicitly disabled in config for local lab mode.
Use bearer token header:

```text
Authorization: Bearer <token>
```

Minimum endpoints:

```text
GET  /api/v1/health
GET  /api/v1/status
POST /api/v1/power/pulse
POST /api/v1/relay/pulse
GET  /api/v1/config
POST /api/v1/config
POST /api/v1/reboot
```

Endpoint semantics:

`GET /api/v1/health`
Returns firmware health, uptime, free heap, Wi-Fi status.

Response:

```json
{
  "ok": true,
  "device": "frostfire",
  "version": "0.1.0",
  "uptimeMs": 123456,
  "wifi": {
    "connected": true,
    "ip": "192.168.1.50",
    "rssi": -51
  },
  "relay": {
    "active": false
  }
}
```

`GET /api/v1/status`
Returns operational state.

Response:

```json
{
  "deviceName": "frostfire",
  "relay": {
    "pin": 5,
    "activeLow": true,
    "active": false,
    "lastPulseAtMs": 120000,
    "lastPulseDurationMs": 500
  },
  "limits": {
    "minPulseMs": 100,
    "maxPulseMs": 3000,
    "defaultPulseMs": 500
  }
}
```

`POST /api/v1/power/pulse`
Primary PC power-button command.
Request body:

```json
{
  "durationMs": 500
}
```

If `durationMs` is absent, use default.
Validate bounds.
Return 202 when accepted.
Return 400 for invalid duration.
Return 401 for missing/invalid auth.
Return 409 if relay is already pulsing.

Response:

```json
{
  "accepted": true,
  "durationMs": 500
}
```

`POST /api/v1/relay/pulse`
Alias for diagnostic relay pulse. Same behavior.

`GET /api/v1/config`
Return non-secret config. Never return Wi-Fi password or auth token.

`POST /api/v1/config`
Allow safe runtime updates only:
`deviceName`
`defaultPulseMs`
`minPulseMs`
`maxPulseMs`
`relayActiveLow`
Do not allow changing relay pin remotely unless implemented with strict validation and immediate safe reinitialization.

`POST /api/v1/reboot`
Authenticated only.
Return response first, then reboot after short delay.

Phase 7: Authentication.
Implement simple token auth.
For initial version, a static token from `secrets.h` is acceptable.
Better version: store token hash in Preferences.
Never expose token through `/config` or `/status`.
Allow `FROSTFIRE_AUTH_DISABLED` only for local development builds.
Document that disabling auth is unsafe on shared networks.

Auth checks:
Only health endpoint may be public.
Status may be public or protected based on config.
All mutating endpoints must be protected.

Phase 8: Optional web UI.
If existing project already has a web UI, preserve and clean it.
If no UI exists, implement only a minimal HTML page after API is stable.
UI must call `/api/v1/status` and `/api/v1/power/pulse`.
Button label should be explicit: “Pulse PC Power Button”.
Avoid label “Toggle” because it is semantically dangerous.
Show relay busy state.
Show Wi-Fi/IP/device/version.
Do not embed secrets in UI.

Phase 9: Logging.
Create lightweight serial logger.
Use levels:
ERROR
WARN
INFO
DEBUG

Log:
boot version
config summary without secrets
Wi-Fi connect/disconnect
IP acquired
API start
relay pulse accepted/rejected
relay forced off
config updated
reboot requested

Never log Wi-Fi password or auth token.

Phase 10: Watchdog and failure behavior.
Main loop must continue calling:
`relay.tick()`
`wifi.tick()` if implemented
`ota.tick()` if enabled
Do not use long blocking delays except during boot diagnostics.
If relay is active and system detects inconsistent state, force OFF.
On reboot, relay must return OFF.

Phase 11: OTA.
Optional compile-time feature.
Disabled by default.
If enabled:
Require password.
Log OTA start/end/error.
Do not allow OTA while relay is active.
Document local-network risk.

Phase 12: MQTT/Home Assistant future extension.
Do not implement unless requested in this iteration.
However, design should not prevent it.
Keep domain layer independent from HTTP server so future MQTT can call the same `RelayController::requestPulse()`.

Potential future MQTT topics:
`frostfire/<device>/status`
`frostfire/<device>/power/pulse`
`frostfire/<device>/availability`

Potential Home Assistant discovery:
Expose as button entity, not switch entity, because PC power button is momentary.

Phase 13: Tests.
Use PlatformIO native/unit tests where practical.
At minimum test pure logic on host if possible:
Relay active-low mapping.
Relay active-high mapping.
Pulse duration validation.
Reject pulse below min.
Reject pulse above max.
Reject concurrent pulse.
Config defaults.
Auth token validation.

Because GPIO is hardware-specific, abstract low-level pin writes behind a tiny interface if needed:

```cpp
class GpioWriter {
public:
  virtual void pinModeOutput(uint8_t pin) = 0;
  virtual void write(uint8_t pin, bool level) = 0;
};
```

For embedded simplicity, only introduce this abstraction if tests are actually implemented. Do not over-engineer.

Phase 14: Manual hardware validation checklist.
Build firmware:
`pio run`

Upload:
`pio run -t upload`

Monitor:
`pio device monitor`

Verify boot:
Relay remains OFF during reset.
Serial logs show firmware name/version.
Wi-Fi connects.
IP is printed.
Health endpoint works.

Call health:

```bash
curl http://<esp32-ip>/api/v1/health
```

Call status:

```bash
curl http://<esp32-ip>/api/v1/status
```

Pulse:

```bash
curl -X POST http://<esp32-ip>/api/v1/power/pulse \
  -H 'Authorization: Bearer <token>' \
  -H 'Content-Type: application/json' \
  -d '{"durationMs":500}'
```

Verify:
Relay clicks once.
Relay turns OFF after duration.
PC behaves like power button was pressed.
Repeated request while busy returns conflict or is ignored safely.
Invalid duration is rejected.
Missing auth is rejected.

Phase 15: Documentation.
Update README.
README must include:
What Frostfire does.
Safety warning.
Hardware wiring.
Relay active LOW/HIGH explanation.
PlatformIO install/build/upload instructions.
Wi-Fi configuration.
API usage with curl.
Troubleshooting.
Security limitations.
License.

Create `docs/wiring.md`.
Include:
ESP32 pin to relay IN.
ESP32 GND to relay GND.
Relay VCC requirement warning.
Relay COM/NO connected to PC power-button header.
Do not connect relay output to mains voltage.
Do not connect ESP32 GPIO directly to motherboard power pins.
Explain that relay contact simulates a button press.

Create `docs/api.md`.
Document every endpoint.
Document auth.
Document error codes.

Create `docs/safety.md`.
Explain:
Default operation is pulse, not toggle.
Relay OFF on boot.
Auth required.
Use only on trusted LAN.
Avoid exposing ESP32 to the public internet.
Use VPN if remote access is needed.

Phase 16: Acceptance criteria.
The coding agent is done only when:

1. `pio run` succeeds.
2. Firmware boots without relay activation.
3. Relay defaults to OFF.
4. Pulse command activates relay for bounded duration only.
5. Relay always returns OFF after pulse.
6. Invalid pulse duration is rejected.
7. Concurrent pulse requests are handled safely.
8. Mutating API endpoints require auth.
9. Wi-Fi credentials are not hardcoded in committed source.
10. JSON responses are valid and stable.
11. README documents PlatformIO workflow.
12. Wiring documentation is explicit.
13. Existing project functionality is preserved or intentionally migrated.
14. No real secrets are committed.
15. Code is modular, readable, and not a monolithic `main.cpp`.

Preferred implementation order:

1. Convert/build existing project with PlatformIO.
2. Add safe relay controller.
3. Add config defaults.
4. Add Wi-Fi connection service.
5. Add health/status endpoints.
6. Add authenticated pulse endpoint.
7. Add tests for relay/config/auth logic.
8. Add docs.
9. Add optional UI only after API is reliable.

Code quality rules:
Keep functions small.
Prefer explicit names.
No magic numbers outside config constants.
No global mutable state except unavoidable Arduino framework objects.
No unbounded `delay()`.
No hidden relay toggles.
No credentials in git.
No public unauthenticated power-control endpoint.
Use `constexpr` for constants.
Use fixed-width integer types.
Use defensive validation for all external input.
Keep heap allocations minimal.
Avoid dynamic `String` abuse in hot paths where easy, but do not micro-optimize prematurely.
Use ArduinoJson for JSON serialization/deserialization rather than manual string concatenation.
Keep domain logic independent from transport layer.

Suggested class sketch:

```cpp
class AppConfig {
public:
  bool load();
  bool save();

  const char* deviceName() const;
  uint8_t relayPin() const;
  bool relayActiveLow() const;
  uint32_t defaultPulseMs() const;
  uint32_t minPulseMs() const;
  uint32_t maxPulseMs() const;
};
```

```cpp
class RelayController {
public:
  explicit RelayController(const AppConfig& config);

  void begin();
  void tick();

  bool requestPulse(uint32_t durationMs);
  bool isActive() const;
  bool isBusy() const;

  uint32_t lastPulseAtMs() const;
  uint32_t lastPulseDurationMs() const;

private:
  void setActive(bool active);
  bool validateDuration(uint32_t durationMs) const;
};
```

```cpp
class WifiService {
public:
  explicit WifiService(const AppConfig& config);

  void begin();
  void tick();

  bool connected() const;
  String ipAddress() const;
  int32_t rssi() const;
};
```

```cpp
class ApiServer {
public:
  ApiServer(AppConfig& config, RelayController& relay, WifiService& wifi);

  void begin();
  void tick();

private:
  bool requireAuth(...);
  void handleHealth(...);
  void handleStatus(...);
  void handlePulse(...);
};
```

Suggested error model:

```json
{
  "error": {
    "code": "invalid_duration",
    "message": "durationMs must be between 100 and 3000"
  }
}
```

Required error codes:
`unauthorized`
`invalid_json`
`invalid_duration`
`relay_busy`
`not_found`
`internal_error`

Security baseline:
This project controls physical power of a PC. Treat every remote command as privileged.
Default to local-network-only.
Do not recommend port forwarding.
For remote access, document VPN/Tailscale/WireGuard as the safe network layer.
Do not implement cloud control in this iteration.

Final deliverables:
Working PlatformIO firmware.
Updated README.
Wiring documentation.
API documentation.
Safety documentation.
Example `secrets.example.h`.
No committed credentials.
Manual test checklist.
Optional minimal web UI if already present or requested.
