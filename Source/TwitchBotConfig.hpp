#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <filesystem>

namespace TwitchBot {

struct TwitchChatConnection
{
  std::string Host;
  unsigned short Port;
  std::string Channel;
  std::string Nick;
};
struct BotCommandCppConfig
{
  std::filesystem::path clangFormatPath;
  std::filesystem::path clangFormatConfigPath;
  std::filesystem::path cppTempPath;
  std::chrono::milliseconds timeout;
};

struct TwitchBotConfig
{
  TwitchChatConnection Connection;
  std::optional<BotCommandCppConfig> CppConfig;
};


auto ReadTwitchBotConfig(std::u8string_view path) -> std::optional<TwitchBotConfig>;

}// namespace TwitchBot