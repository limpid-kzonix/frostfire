#pragma once

#include <cctype>
#include <string_view>

namespace AuthPolicy
{
constexpr const char *kDefaultTokenSentinel = "change-me";

inline constexpr std::string_view kBearerPrefix = "Bearer ";

inline std::string_view trimWhitespace(std::string_view value);

inline bool isConfiguredTokenValid(std::string_view token)
{
  const auto trimmed = trimWhitespace(token);
  return !trimmed.empty() && trimmed != kDefaultTokenSentinel;
}

inline std::string_view trimWhitespace(std::string_view value)
{
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
  {
    value.remove_prefix(1);
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
  {
    value.remove_suffix(1);
  }
  return value;
}

inline std::string_view extractBearerToken(std::string_view authorizationHeader)
{
  const auto header = trimWhitespace(authorizationHeader);
  if (header.size() < kBearerPrefix.size())
  {
    return {};
  }

  for (size_t index = 0; index < kBearerPrefix.size(); ++index)
  {
    if (std::tolower(static_cast<unsigned char>(header[index])) != std::tolower(static_cast<unsigned char>(kBearerPrefix[index])))
    {
      return {};
    }
  }

  return trimWhitespace(header.substr(kBearerPrefix.size()));
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
  if (provided.empty())
  {
    return false;
  }

  return provided == trimWhitespace(configuredToken);
}
} // namespace AuthPolicy
