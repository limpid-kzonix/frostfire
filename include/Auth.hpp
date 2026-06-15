#pragma once

#include <Arduino.h>

class Auth
{
public:
  explicit Auth(const String &token);

  bool isAuthorized(const String &authorizationHeader) const;

private:
  String authToken() const;
  bool isAuthDisabled() const;
  String extractBearerToken(const String &authorizationHeader) const;

private:
  String _token;
};
