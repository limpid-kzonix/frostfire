#pragma once

#include <cstdint>

namespace PulsePolicy
{
enum class ValidationError
{
  None,
  MinPulseMsInvalid,
  MaxPulseMsInvalid,
  MinAfterMax,
  DefaultOutOfRange
};

inline constexpr bool isDurationWithinBounds(uint32_t durationMs, uint32_t minPulseMs, uint32_t maxPulseMs)
{
  return durationMs >= minPulseMs && durationMs <= maxPulseMs;
}

inline constexpr bool relayOutputHighForActive(bool active, bool relayActiveLow)
{
  return active != relayActiveLow;
}

inline constexpr ValidationError validatePulseConfig(uint32_t defaultPulseMs, uint32_t minPulseMs, uint32_t maxPulseMs)
{
  if (minPulseMs == 0)
  {
    return ValidationError::MinPulseMsInvalid;
  }
  if (maxPulseMs == 0)
  {
    return ValidationError::MaxPulseMsInvalid;
  }
  if (minPulseMs > maxPulseMs)
  {
    return ValidationError::MinAfterMax;
  }
  if (defaultPulseMs < minPulseMs || defaultPulseMs > maxPulseMs)
  {
    return ValidationError::DefaultOutOfRange;
  }
  return ValidationError::None;
}

inline constexpr const char *validationErrorMessage(ValidationError error)
{
  switch (error)
  {
  case ValidationError::MinPulseMsInvalid:
    return "minPulseMs must be greater than 0";
  case ValidationError::MaxPulseMsInvalid:
    return "maxPulseMs must be greater than 0";
  case ValidationError::MinAfterMax:
    return "minPulseMs must be <= maxPulseMs";
  case ValidationError::DefaultOutOfRange:
    return "defaultPulseMs must be between minPulseMs and maxPulseMs";
  default:
    return "";
  }
}
}
