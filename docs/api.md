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

### `GET /api/v1/status`

Returns relay and limit state.

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
