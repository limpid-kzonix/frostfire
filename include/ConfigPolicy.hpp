#pragma once

#include <cstdint>

#include "PulsePolicy.hpp"

namespace ConfigPolicy
{
constexpr const char *kDefaultDeviceName = "frostfire";
constexpr uint8_t kDefaultRelayPin = 5;
constexpr uint8_t kMinValidRelayPin = 0;
constexpr uint8_t kMaxValidRelayPin = 33;
constexpr bool kDefaultRelayActiveLow = true;
constexpr uint32_t kDefaultPulseMs = 500;
constexpr uint32_t kDefaultMinPulseMs = 100;
constexpr uint32_t kDefaultMaxPulseMs = 3000;
constexpr bool kDefaultApiEnabled = true;
constexpr bool kDefaultOtaEnabled = false;
constexpr bool kDefaultStaticIpEnabled = false;

inline constexpr bool isInvalidRelayPin(uint8_t relayPin)
{
  return relayPin == 0 || relayPin == 2 || relayPin == 12 || relayPin == 15 ||
         (relayPin >= 6 && relayPin <= 11) ||
         relayPin > kMaxValidRelayPin;
}

inline constexpr bool isValidRelayPin(uint8_t relayPin)
{
  return relayPin >= kMinValidRelayPin && relayPin <= kMaxValidRelayPin && !isInvalidRelayPin(relayPin);
}

inline constexpr bool isValidRelayPulseConfig(uint32_t defaultPulseMs, uint32_t minPulseMs, uint32_t maxPulseMs)
{
  return PulsePolicy::validatePulseConfig(defaultPulseMs, minPulseMs, maxPulseMs) == PulsePolicy::ValidationError::None;
}
} // namespace ConfigPolicy
