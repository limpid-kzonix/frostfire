#pragma once

#include <Arduino.h>

class Logger
{
public:
  enum class Level : uint8_t
  {
    Error = 0,
    Warn = 1,
    Info = 2,
    Debug = 3
  };

  void begin(uint32_t baudRate = 115200);
  void setLevel(Level level);
  void error(const String &message) const;
  void warn(const String &message) const;
  void info(const String &message) const;
  void debug(const String &message) const;

  bool enabled(Level level) const;

private:
  void log(Level level, const String &message) const;

private:
  Level _level = Level::Info;
};

extern Logger logger;
