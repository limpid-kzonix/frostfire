#pragma once

#include <string_view>

namespace AuthPolicy
{
constexpr const char *kDefaultTokenSentinel = "change-me";

inline constexpr std::string_view kBearerPrefix = "Bearer ";

inline bool isConfiguredTokenValid(std::string_view token)
{
  return !token.empty() && token != kDefaultTokenSentinel;
}

inline std::string_view extractBearerToken(std::string_view authorizationHeader)
{
  if (authorizationHeader.size() < kBearerPrefix.size())
  {
    return {};
  }

  const char *prefixData = kBearerPrefix.data();
  for (size_t index = 0; index < kBearerPrefix.size(); ++index)
  {
    if (authorizationHeader[index] != prefixData[index])
    {
      return {};
    }
  }

  return authorizationHeader.substr(kBearerPrefix.size());
}

inline bool isAuthorized(std::string_view configuredToken, std::string_view authorizationHeader, bool authDisabled)
{
  if (authDisabled)
  {
    return true;
  }

  if (!isConfiguredTokenValid(configuredToken))
  {
    return false;
  }

  const std::string_view provided = extractBearerToken(authorizationHeader);
  return provided == configuredToken;
}
} // namespace AuthPolicy
