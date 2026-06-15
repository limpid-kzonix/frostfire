#pragma once

#ifndef FROSTFIRE_FW_NAME
#define FROSTFIRE_FW_NAME "Frostfire"
#endif

#ifndef FROSTFIRE_FW_VERSION
#define FROSTFIRE_FW_VERSION "0.1.0"
#endif

inline constexpr const char *getFirmwareName()
{
  return FROSTFIRE_FW_NAME;
}

inline constexpr const char *getFirmwareVersion()
{
  return FROSTFIRE_FW_VERSION;
}
