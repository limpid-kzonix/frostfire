#include <iostream>
#include <string_view>
#include <cstdint>

#include "AuthPolicy.hpp"

int main()
{
  uint32_t failures = 0;

  auto expect = [&](bool condition, const char *message)
  {
    if (!condition)
    {
      ++failures;
      std::cout << "FAILED: " << message << "\n";
    }
  };

  expect(AuthPolicy::isAuthorized("change-me", "Bearer abc", false) == false, "default token is rejected");
  expect(AuthPolicy::isAuthorized("", "Bearer abc", false) == false, "empty token is rejected");
  expect(AuthPolicy::isAuthorized("super-secret", "Bearer super-secret", false) == true, "valid bearer token accepted");
  expect(AuthPolicy::isAuthorized("super-secret", "Token super-secret", false) == false, "invalid prefix rejected");
  expect(AuthPolicy::isAuthorized("super-secret", "  Bearer   super-secret  ", false) == true, "token with extra spaces accepted");
  expect(AuthPolicy::isAuthorized("super-secret", "bearer super-secret", false) == true, "case-insensitive bearer accepted");
  expect(AuthPolicy::isAuthorized("super-secret", "", false) == false, "empty header rejected");
  expect(AuthPolicy::isAuthorized("  super-secret  ", "Bearer super-secret", false) == true, "configured token with spaces is normalized");
  expect(AuthPolicy::isConfiguredTokenValid("   ") == false, "whitespace token rejected");

  expect(AuthPolicy::isAuthorized("super-secret", "", true) == true, "auth disabled always accepts");
  expect(AuthPolicy::isAuthorized("", "", true) == true, "auth disabled always accepts when token missing");

  expect(AuthPolicy::extractBearerToken("Bearer super-secret") == std::string_view("super-secret"), "extractBearerToken parses well");
  expect(AuthPolicy::extractBearerToken("Token super-secret") == std::string_view(""), "extractBearerToken requires Bearer prefix");
  expect(AuthPolicy::extractBearerToken("  Bearer   super-secret  ") == std::string_view("super-secret"), "extractBearerToken trims whitespace");

  if (failures > 0)
  {
    std::cout << "Auth policy tests failed: " << failures << "\n";
    return 1;
  }

  std::cout << "Auth policy tests passed\n";
  return 0;
}
