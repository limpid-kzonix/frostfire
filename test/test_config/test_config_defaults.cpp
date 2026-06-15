#include <iostream>
#include <string>

#include "ConfigPolicy.hpp"
#include "PulsePolicy.hpp"

int main()
{
  uint32_t failures = 0;

  auto expect = [&](bool condition, const char *message)
  {
    if (!condition)
    {
      ++failures;
      std::cout << "FAILED: " << message << "\n";
    }
  };

  expect(std::string(ConfigPolicy::kDefaultDeviceName) == "frostfire", "default device name is frostfire");
  expect(ConfigPolicy::kDefaultRelayPin == 5, "default relay pin is 5");
  expect(ConfigPolicy::kDefaultRelayActiveLow == true, "default relay active level is active-low");
  expect(ConfigPolicy::kDefaultPulseMs == 500, "default pulse is 500ms");
  expect(ConfigPolicy::kDefaultMinPulseMs == 100, "minimum pulse is 100ms");
  expect(ConfigPolicy::kDefaultMaxPulseMs == 3000, "maximum pulse is 3000ms");
  expect(ConfigPolicy::kDefaultApiEnabled == true, "api enabled by default");
  expect(ConfigPolicy::kDefaultOtaEnabled == false, "ota disabled by default");
  expect(ConfigPolicy::kDefaultStaticIpEnabled == false, "static IP disabled by default");

  expect(ConfigPolicy::isValidRelayPulseConfig(500, 100, 3000), "default relay pulse bounds are valid");
  expect(!ConfigPolicy::isValidRelayPulseConfig(50, 100, 3000), "invalid default pulse rejected");

  expect(PulsePolicy::isDurationWithinBounds(500, ConfigPolicy::kDefaultMinPulseMs, ConfigPolicy::kDefaultMaxPulseMs), "duration within default bounds accepted");
  expect(!PulsePolicy::isDurationWithinBounds(99, ConfigPolicy::kDefaultMinPulseMs, ConfigPolicy::kDefaultMaxPulseMs), "duration below minimum rejected by defaults");
  expect(!PulsePolicy::isDurationWithinBounds(3001, ConfigPolicy::kDefaultMinPulseMs, ConfigPolicy::kDefaultMaxPulseMs), "duration above maximum rejected by defaults");

  if (failures > 0)
  {
    std::cout << "Config policy tests failed: " << failures << "\n";
    return 1;
  }

  std::cout << "Config policy tests passed\n";
  return 0;
}
