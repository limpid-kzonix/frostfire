# Frostfire Safety

- Frostfire always sends a bounded pulse by default. It never keeps relay ON indefinitely.
- Relay is OFF during boot and during firmware initialization before networking starts.
- Keep the device on a trusted local LAN.
- Disable auth only for local development (via `FROSTFIRE_AUTH_DISABLED`) and never on shared networks.
- Never expose this firmware directly to the public internet.
- If remote access is required, place ESP32 behind VPN/Tailscale/WireGuard.
- OTA is optional and must remain LAN-only unless protected by trusted network controls.
- Power-cable wiring must only use relay contacts across the motherboard power-button header.
- Do not route AC mains through the ESP32 relay output.
