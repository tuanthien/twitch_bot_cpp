#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace TwitchBot {
struct HttpServerConfig
{
  std::u8string Host;
  unsigned short Port;
  std::u8string RootDoc;
};
struct WebSocketConfig
{
  int QueueSize;
};
struct ChatServerConfig
{
  HttpServerConfig Http;
  WebSocketConfig WebSocket;
};

auto ReadServerConfig(std::u8string_view path) -> std::optional<ChatServerConfig>;
}// namespace TwitchBot