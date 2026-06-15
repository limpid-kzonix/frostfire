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

namespace
{
void logConfigSummary()
{
  const auto boolText = [](bool value)
  {
    return value ? "true" : "false";
  };

  logger.info("config summary:");
  logger.info(String("  deviceName=") + config.deviceName() +
              ", relayPin=" + String(config.relayPin()) +
              ", relayActiveLow=" + boolText(config.relayActiveLow()));
  logger.info(String("  pulse: default=") + String(config.defaultPulseMs()) +
              "ms, min=" + String(config.minPulseMs()) +
              "ms, max=" + String(config.maxPulseMs()) + "ms");
  logger.info(String("  apiEnabled=") + boolText(config.apiEnabled()) +
              ", otaEnabled=" + boolText(config.otaEnabled()));
  logger.info(String("  wifiSsid=") + (config.wifiSsid().isEmpty() ? "<empty>" : config.wifiSsid()) +
              ", staticIpEnabled=" + boolText(config.staticIpEnabled()));
}
}

void setup()
{
  logger.begin(115200);
  logger.info("starting frostfire firmware");
  logger.info(String(getFirmwareName()) + " " + String(getFirmwareVersion()));

  if (!config.load())
  {
    logger.warn("configuration invalid, using defaults");
  }
  logConfigSummary();
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
