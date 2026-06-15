#include <Arduino.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

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
  if (isAuthDisabled())
  {
    return true;
  }

  const String token = authToken();
  if (token.isEmpty() || token == "change-me")
  {
    return false;
  }

  const String provided = extractBearerToken(authorizationHeader);
  return provided == token;
}
