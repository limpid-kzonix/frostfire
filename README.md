# Frostfire

Frostfire is ESP32 firmware that safely emulates a PC motherboard power-button press via a relay.

## Safety model

- Commands are bounded pulses only: the relay turns on for a fixed duration and then returns off.
- Relay defaults to OFF during boot/restart and after every pulse.
- Mutating API endpoints require token auth.
- This is for trusted local networks only.

## Hardware

- ESP32 development board
- Single-channel relay module
- One GPIO to relay input
- Relay common/NO contacts across PC power-button header

## Wiring

See [`docs/wiring.md`](docs/wiring.md).

## Repository layout

- `include/` firmware headers
- `src/` firmware sources
- `test/` firmware tests (planned)
- `docs/` API/safety/wiring docs

## Setup

1. Install PlatformIO and clone this repository.
2. Create credentials:

```bash
cp include/secrets.example.h include/secrets.h
```

3. Edit `include/secrets.h`.
4. Build:

```bash
pio run
```

5. Upload:

```bash
pio run -t upload
```

6. Monitor:

```bash
pio device monitor
```

## Configuration

Runtime configuration is stored in ESP32 `Preferences`.
Defaults:

- `deviceName = frostfire`
- `relayPin = 5`
- `relayActiveLow = true`
- `defaultPulseMs = 500`
- `minPulseMs = 100`
- `maxPulseMs = 3000`
- `apiEnabled = true`
- `otaEnabled = false`

See API for runtime updates.

## API

See [`docs/api.md`](docs/api.md).

## Useful API calls

```bash
curl http://<ip>/api/v1/health
curl http://<ip>/api/v1/status
curl -X POST http://<ip>/api/v1/power/pulse \
  -H 'Authorization: Bearer <token>' \
  -H 'Content-Type: application/json' \
  -d '{"durationMs":500}'
```

### Example: check config

```bash
curl http://<ip>/api/v1/config
```

### Example: safely tune pulse duration limits

```bash
curl -X POST http://<ip>/api/v1/config \
  -H 'Authorization: Bearer <token>' \
  -H 'Content-Type: application/json' \
  -d '{"defaultPulseMs":500,"minPulseMs":100,"maxPulseMs":3000}'
```

## Troubleshooting

- API always returns 401: confirm your token header is `Authorization: Bearer <token>` and that `FROSTFIRE_API_TOKEN` in `include/secrets.h` is not `change-me`.
- PC does not power on: verify relay polarity (`relayActiveLow`) matches your relay module and wiring.
- Relay clicks but power does not toggle: check relay contact type and PC motherboard power-button header wiring (COM and NO only).
- Wi-Fi not connecting: verify credentials in `include/secrets.h` and that the SSID is reachable by ESP32.
- Frequent `relay_busy` responses: the relay is still in a pulse window; wait for `durationMs` to elapse before retrying.

## Security limitations

- This firmware intentionally defaults to local-network use.
- Do not expose it directly to WAN, port forwarding, or public DNS names.
- Disable auth only for local lab development and never on shared or unattended networks.
- For remote access, use a trusted VPN layer such as Tailscale/WireGuard.

## License

Licensed under the [MIT License](LICENSE).

## Validation

From firmware perspective:

- `pio run`
- power cycle with relay disconnected from PCB pins and verify it stays off during boot
- call `GET /api/v1/health` and `GET /api/v1/status`
- send a pulse request and confirm it returns to OFF after requested duration
- verify invalid payloads and missing auth return errors as defined in [docs/api.md](docs/api.md)

Host test (current environment):

```bash
g++ -std=c++17 -Iinclude \
  test/test_relay_controller/test_pulse_policy.cpp \
  test/test_config/test_config_defaults.cpp \
  test/test_auth/test_auth_policy.cpp \
  -o /tmp/frostfire_tests && /tmp/frostfire_tests
```

## Validation checklist

- `pio run`
- Power cycles leave relay OFF
- `POST /api/v1/power/pulse` returns 202 and relay deactivates after configured duration
- Invalid duration returns 400
- Busy pulse returns 409
- Unauthorized control returns 401

## Security

- Never commit real secrets; only `include/secrets.example.h` is versioned.
- Do not expose API over public networks.
- Prefer VPN access for remote use.
- For lab-only local builds only, you can disable auth by defining `FROSTFIRE_AUTH_DISABLED`.

## License

MIT.
