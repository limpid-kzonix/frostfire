#include "Logger.hpp"

Logger logger;

void Logger::begin(uint32_t baudRate)
{
  Serial.begin(baudRate);
  while (!Serial && millis() < 3000)
  {
    delay(10);
  }
}

void Logger::setLevel(Level level)
{
  _level = level;
}

bool Logger::enabled(Level level) const
{
  return static_cast<uint8_t>(level) <= static_cast<uint8_t>(_level);
}

void Logger::log(Level level, const String &message) const
{
  if (!enabled(level))
  {
    return;
  }
  const char *prefix = "";
  if (level == Level::Error)
  {
    prefix = "[ERR] ";
  }
  else if (level == Level::Warn)
  {
    prefix = "[WRN] ";
  }
  else if (level == Level::Info)
  {
    prefix = "[INF] ";
  }
  else
  {
    prefix = "[DBG] ";
  }
  Serial.print(prefix);
  Serial.println(message);
}

void Logger::error(const String &message) const
{
  log(Level::Error, message);
}

void Logger::warn(const String &message) const
{
  log(Level::Warn, message);
}

void Logger::info(const String &message) const
{
  log(Level::Info, message);
}

void Logger::debug(const String &message) const
{
  log(Level::Debug, message);
}

