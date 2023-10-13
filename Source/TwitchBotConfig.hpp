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
};

struct TwitchBotConfig
{
  TwitchChatConnection Connection;
  BotCommandCppConfig CppConfig;
};


auto ReadTwitchBotConfig(std::u8string_view path) -> std::optional<TwitchBotConfig>;

}// namespace TwitchBot