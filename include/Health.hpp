#pragma once

#include <Arduino.h>

struct HealthState
{
  bool ok;
  uint32_t uptimeMs;
  uint32_t freeHeap;
};
