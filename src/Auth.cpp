#include <Arduino.h>

#include <string_view>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#include "AuthPolicy.hpp"
#include "Auth.hpp"

Auth::Auth(const String &token)
    : _token(token)
{
}

bool Auth::isAuthDisabled() const
{
#ifdef FROSTFIRE_AUTH_DISABLED
  return true;
#else
  return false;
#endif
}

String Auth::extractBearerToken(const String &authorizationHeader) const
{
  const String prefix = "Bearer ";
  if (!authorizationHeader.startsWith(prefix))
  {
    return "";
  }
  const String raw = authorizationHeader.substring(prefix.length());
  return raw;
}

String Auth::authToken() const
{
  if (!_token.isEmpty())
  {
    return _token;
  }

#ifdef FROSTFIRE_API_TOKEN
  return FROSTFIRE_API_TOKEN;
#else
  return "change-me";
#endif
}

bool Auth::isAuthorized(const String &authorizationHeader) const
{
  const String configured = authToken();
  const std::string_view configuredToken = configured.c_str();
  const std::string_view providedHeader = authorizationHeader.c_str();
  return AuthPolicy::isAuthorized(configuredToken, providedHeader, isAuthDisabled());
}
