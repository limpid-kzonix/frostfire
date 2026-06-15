#include "ApiServer.hpp"
#include "BuildInfo.hpp"
#include "Logger.hpp"
#include "RelayController.hpp"
#include "OtaService.hpp"
#include "WifiService.hpp"
#include <Arduino.h>

AppConfig config;
RelayController relay(config);
WifiService wifi(config);
ApiServer api(config, relay, wifi, ""); // token can be supplied via include/secrets.h
OtaService ota(config, relay);

void setup()
{
  logger.begin(115200);
  logger.info("starting frostfire firmware");
  logger.info(String(getFirmwareName()) + " " + String(getFirmwareVersion()));

  if (!config.load())
  {
    logger.warn("configuration invalid, using defaults");
  }
  relay.begin();
  wifi.begin();
  api.begin();
  ota.begin();

  logger.info("system initialized");
}

void loop()
{
  relay.tick();
  wifi.tick();
  api.tick();
  ota.tick();
}
