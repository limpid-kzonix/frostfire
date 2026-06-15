#include "Logger.hpp"
#include "OtaService.hpp"

#ifdef FROSTFIRE_OTA_ENABLED
#include <ArduinoOTA.h>
#endif

OtaService::OtaService(const AppConfig &config, const RelayController &relay)
    : _config(config),
      _relay(relay),
#ifdef FROSTFIRE_OTA_PASSWORD
      _password(FROSTFIRE_OTA_PASSWORD),
#else
      _password(""),
#endif
      _initialized(false),
      _pausedByRelay(false)
{
}

void OtaService::begin()
{
#ifdef FROSTFIRE_OTA_ENABLED
  if (!_config.otaEnabled())
  {
    logger.info("ota disabled by config");
    return;
  }

  if (_password.isEmpty())
  {
    logger.warn("ota enabled but no FROSTFIRE_OTA_PASSWORD configured");
    return;
  }

  ArduinoOTA.setHostname(_config.deviceName().c_str());
  ArduinoOTA.setPassword(_password.c_str());
  configureCallbacks();
  ArduinoOTA.begin();
  _initialized = true;
  logger.info("ota service started");
#else
  if (_config.otaEnabled())
  {
    logger.warn("ota requested but build disabled (define FROSTFIRE_OTA_ENABLED to enable)");
  }
  else
  {
    logger.debug("ota disabled");
  }
  (void)_relay;
#endif
}

void OtaService::tick()
{
#ifdef FROSTFIRE_OTA_ENABLED
  if (!_initialized)
  {
    return;
  }

  if (_relay.isActive())
  {
    if (!_pausedByRelay)
    {
      logger.warn("ota handling paused while relay is active");
      _pausedByRelay = true;
    }
    return;
  }
  _pausedByRelay = false;

  ArduinoOTA.handle();
#else
  (void)_relay;
#endif
}

#ifdef FROSTFIRE_OTA_ENABLED
void OtaService::configureCallbacks()
{
  ArduinoOTA.onStart([]()
                     { logger.info("ota update started"); });

  ArduinoOTA.onEnd([]()
                   { logger.info("ota update ended"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
    const uint32_t percent = total ? (progress * 100UL) / total : 0;
    logger.debug("ota progress: " + String(percent) + "%");
  });

  ArduinoOTA.onError([](ota_error_t error)
                     {
    const char *message = "unknown";
    if (error == OTA_AUTH_ERROR)
    {
      message = "auth error";
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      message = "begin error";
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      message = "connect error";
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      message = "receive error";
    }
    else if (error == OTA_END_ERROR)
    {
      message = "end error";
    }
    logger.error("ota error: " + String(message)); });
}
#endif
