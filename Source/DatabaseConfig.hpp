#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <filesystem>

#include "Database.hpp"

namespace TwitchBot {

struct DatabaseConfig
{
  std::u8string Host;
  std::uint16_t Port{3306U};
  std::u8string Username;
  std::u8string Password;
  std::u8string Database;
};


auto ReadDatabaseConfig(std::u8string_view path) -> std::optional<DatabaseConfig>;

}// namespace TwitchBot