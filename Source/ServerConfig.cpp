#include "ServerConfig.hpp"
#include "Conversion.hpp"

#include <limits>

#include <fmt/core.h>
#include <fmt/format.h>
#include <simdjson.h>


namespace TwitchBot {

auto ReadServerConfig(std::u8string_view path) -> std::optional<ChatServerConfig>
{
  namespace json = simdjson;

  auto jsonPath = std::string_view(reinterpret_cast<const char *>(path.data()), path.size());

  json::padded_string jsonStr;
  auto error = json::padded_string::load(jsonPath).get(jsonStr);
  if (error) {
    fmt::print("Error: fail to load {}", jsonPath);
    return std::nullopt;
  }

  json::ondemand::parser parser;
  json::ondemand::document jsonDoc;
  error = parser.iterate(jsonStr).get(jsonDoc);
  if (error) {
    fmt::print("Error: {} file seem to be invalid json", jsonPath);
    return std::nullopt;
  }

  json::ondemand::object httpObj;
  error = jsonDoc["http"].get_object().get(httpObj);
  if (error) {
    fmt::print("Error: http field is not an object");
    return std::nullopt;
  }

  std::string_view host;
  error = httpObj["host"].get_string().get(host);
  if (error) {
    fmt::print("Error: host field is not a string");
    return std::nullopt;
  }

  int64_t portInt64;
  error = httpObj["port"].get_int64().get(portInt64);
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

  std::string_view rootDoc;
  error = httpObj["root_doc"].get_string().get(rootDoc);
  if (error) {
    fmt::print("Error: root_doc field is not a string");
    return std::nullopt;
  }
  json::ondemand::object websocketObj;
  error = jsonDoc["websocket"].get_object().get(websocketObj);
  if (error) {
    fmt::print("Error: websocket field is not an object");
    return std::nullopt;
  }
  int64_t queueSizeInt64;
  error = websocketObj["queue_size"].get_int64().get(queueSizeInt64);
  if (error) {
    fmt::print("Error: queue_size field is not int64");
    return std::nullopt;
  }

  fmt::print("Http:\n\tHost: {}\n\tPort: {}\n\tRoot document: {}\n", host, port, rootDoc);
  fmt::print("Websocket:\n\tQueue size: {}\n", queueSizeInt64);

  return ChatServerConfig{HttpServerConfig{to_u8string(host), port, to_u8string(rootDoc)}, WebSocketConfig{static_cast<int>(queueSizeInt64)}};
}

}// namespace TwitchBot