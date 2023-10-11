#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace TwitchBot {

struct TwitchChatConnection
{
  std::string Host;
  unsigned short Port;
  std::string Channel;
  std::string Nick;
};
auto ReadTwitchBotConfig(std::u8string_view path) -> std::optional<TwitchChatConnection>;

}// namespace TwitchBot