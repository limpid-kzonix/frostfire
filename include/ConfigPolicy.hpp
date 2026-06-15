#pragma once

#include <cstdint>

#include "PulsePolicy.hpp"

namespace ConfigPolicy
{
constexpr const char *kDefaultDeviceName = "frostfire";
constexpr uint8_t kDefaultRelayPin = 5;
constexpr bool kDefaultRelayActiveLow = true;
constexpr uint32_t kDefaultPulseMs = 500;
constexpr uint32_t kDefaultMinPulseMs = 100;
constexpr uint32_t kDefaultMaxPulseMs = 3000;
constexpr bool kDefaultApiEnabled = true;
constexpr bool kDefaultOtaEnabled = false;
constexpr bool kDefaultStaticIpEnabled = false;

inline constexpr bool isValidRelayPulseConfig(uint32_t defaultPulseMs, uint32_t minPulseMs, uint32_t maxPulseMs)
{
  return PulsePolicy::validatePulseConfig(defaultPulseMs, minPulseMs, maxPulseMs) == PulsePolicy::ValidationError::None;
}
} // namespace ConfigPolicy
