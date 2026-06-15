#include <cstdint>
#include <iostream>

#include "PulsePolicy.hpp"

int main()
{
  uint32_t failed = 0;

  auto expect = [&](bool ok, const char *description)
  {
    if (!ok)
    {
      ++failed;
      std::cout << "FAILED: " << description << "\n";
    }
  };

  expect(PulsePolicy::validatePulseConfig(500, 100, 3000) == PulsePolicy::ValidationError::None, "valid pulse config");
  expect(PulsePolicy::validatePulseConfig(0, 100, 3000) == PulsePolicy::ValidationError::DefaultOutOfRange, "default below minimum invalid");
  expect(PulsePolicy::validatePulseConfig(500, 0, 3000) == PulsePolicy::ValidationError::MinPulseMsInvalid, "zero minimum invalid");
  expect(PulsePolicy::validatePulseConfig(500, 100, 0) == PulsePolicy::ValidationError::MaxPulseMsInvalid, "zero maximum invalid");
  expect(PulsePolicy::validatePulseConfig(50, 100, 200) == PulsePolicy::ValidationError::DefaultOutOfRange, "default above config range invalid");
  expect(PulsePolicy::validatePulseConfig(100, 300, 200) == PulsePolicy::ValidationError::MinAfterMax, "min above max invalid");

  expect(PulsePolicy::isDurationWithinBounds(500, 100, 3000), "bounded duration accepted");
  expect(!PulsePolicy::isDurationWithinBounds(99, 100, 3000), "too short duration rejected");
  expect(!PulsePolicy::isDurationWithinBounds(3001, 100, 3000), "too long duration rejected");

  // relayActiveLow=true -> active LOW
  expect(PulsePolicy::relayOutputHighForActive(false, true), "active-low relay off maps to HIGH output");
  expect(!PulsePolicy::relayOutputHighForActive(true, true), "active-low relay on maps to LOW output");

  // relayActiveLow=false -> active HIGH
  expect(!PulsePolicy::relayOutputHighForActive(false, false), "active-high relay off maps to LOW output");
  expect(PulsePolicy::relayOutputHighForActive(true, false), "active-high relay on maps to HIGH output");

  if (failed > 0)
  {
    std::cout << "Pulse policy tests failed: " << failed << "\n";
    return 1;
  }

  std::cout << "Pulse policy tests passed\n";
  return 0;
}
