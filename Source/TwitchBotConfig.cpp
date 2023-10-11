#include "TwitchBotConfig.hpp"

#include <limits>

#include <fmt/core.h>
#include <fmt/format.h>
#include <simdjson.h>

namespace TwitchBot {

auto ReadTwitchBotConfig(std::u8string_view path) -> std::optional<TwitchChatConnection>
{
  namespace json = simdjson;

  auto twitchJsonFilePath = std::string_view(reinterpret_cast<const char *>(path.data()), path.size());

  json::padded_string twitchJsonStr;
  auto error = json::padded_string::load(twitchJsonFilePath).get(twitchJsonStr);
  if (error) {
    fmt::print("Error: fail to load {}", twitchJsonFilePath);
    return std::nullopt;
  }

  json::ondemand::parser parser;
  json::ondemand::document twitchJson;
  error = parser.iterate(twitchJsonStr).get(twitchJson);
  if (error) {
    fmt::print("Error: {} file seem to be invalid json", twitchJsonFilePath);
    return std::nullopt;
  }
  std::string_view host;
  error = twitchJson["host"].get_string().get(host);
  if (error) {
    fmt::print("Error: host field is not a string");
    return std::nullopt;
  }
  int64_t portInt64;
  error = twitchJson["port"].get_int64().get(portInt64);
  if (error) {
    fmt::print("Error: port field is not int64");
    return std::nullopt;
  }
  if (
    portInt64 < std::numeric_limits<unsigned short>::min() || portInt64 > std::numeric_limits<unsigned short>::max()) {
    fmt::print("Error: invalid port value");
    return std::nullopt;
  }
  unsigned short port = static_cast<unsigned short>(portInt64);

  std::string_view nick;
  error = twitchJson["nick"].get_string().get(nick);
  if (error) {
    fmt::print("Error: nick field is not a string");
    return std::nullopt;
  }

  std::string_view channel;
  error = twitchJson["channel"].get_string().get(channel);
  if (error) {
    fmt::print("Error: channel field is not a string");
    return std::nullopt;
  }

  fmt::print("Twitch:\n\tHost: {}\n\tPort: {}\n\tChannel: {}\n\tNick: {}\n", host, port, channel, nick);


  return TwitchChatConnection{std::string(host), port, std::string(channel), std::string(nick)};
}

}// namespace TwitchBot