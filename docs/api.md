# Frostfire API

All endpoints are under `/api/v1`.

## Authentication

Mutating endpoints require:

```
Authorization: Bearer <token>
```

`GET /api/v1/health` is public by default.

Error response shape:

```json
{
  "error": {
    "code": "unauthorized",
    "message": "unauthorized access for ..."
  }
}
```

## Endpoints

### `GET /api/v1/health`

Returns firmware health and network status.

Response example:

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
  },
  "heapFree": 196000
}
```

### `GET /api/v1/status`

Returns relay and limit state.

Response example:

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
  },
  "wifi": {
    "ssid": "my-wifi",
    "staticIpEnabled": false,
    "staticIp": "",
    "gateway": "",
    "subnet": "",
    "dns": ""
  },
  "feature": {
    "apiEnabled": true,
    "otaEnabled": false,
    "version": "0.1.0",
    "device": "Frostfire"
  }
}
```

### `POST /api/v1/power/pulse`

Request body:

```json
{"durationMs": 500}
```

- `durationMs` optional, uses default configured pulse duration.
- 202 accepted when the pulse is scheduled.
- 400 on invalid duration or malformed JSON.
- 409 if relay is already pulsing.

Response:

```json
{"accepted":true,"durationMs":500}
```

Error response example:

```json
{
  "error": {
    "code": "relay_busy",
    "message": "relay already pulsing"
  }
}
```

### `POST /api/v1/relay/pulse`

Alias to `/api/v1/power/pulse`.

### `GET /api/v1/config`

Returns non-secret configuration.

### `POST /api/v1/config`

Allowed keys:

- `deviceName` (string)
- `defaultPulseMs` (positive integer)
- `minPulseMs` (positive integer)
- `maxPulseMs` (positive integer)
- `relayActiveLow` (boolean)

`relayPin` cannot be changed remotely.

### `POST /api/v1/reboot`

Returns `202` and reboots shortly after response.

## Error codes

- `unauthorized`
- `invalid_json`
- `invalid_duration`
- `invalid_config`
- `relay_busy`
- `not_found`
- `internal_error`

### Error response format

```json
{
  "error": {
    "code": "invalid_duration",
    "message": "durationMs must be within configured bounds"
  }
}
```
