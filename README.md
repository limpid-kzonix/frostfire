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
