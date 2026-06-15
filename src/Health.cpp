#include "Health.hpp"

HealthState sampleHealthState()
{
  HealthState state;
  state.ok = true;
  state.uptimeMs = millis();
  state.freeHeap = ESP.getFreeHeap();
  return state;
}
