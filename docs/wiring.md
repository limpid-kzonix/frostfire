# Frostfire Wiring

## Relay wiring

- ESP32 GPIO to relay module **IN** input.
- ESP32 GND to relay module **GND**.
- Relay module supply (VCC) must match relay module specification. Most 5V modules need 5V. Do not connect directly to motherboard power rails.
- Relay contact:
  - Relay COM and NO across the PC motherboard power button header.
  - Do **not** connect relay output to mains voltage.
  - Do **not** drive motherboard power pins directly from ESP32 GPIO.

## GPIO pin recommendations

- Recommended default relay GPIO: `5` (safe for many ESP32 dev boards).
- Do not use boards’ strapping pins as defaults unless your board layout confirms they are safe for your specific board.

## Active level

- `relayActiveLow = true` means relay OFF = HIGH, ON = LOW.
- `relayActiveLow = false` means relay OFF = LOW, ON = HIGH.

## Power safety

- Keep the relay off at boot by firmware default state.
- Keep `RELAY` physically isolated from user-accessible mains wiring.
- Never keep relay energized continuously.
