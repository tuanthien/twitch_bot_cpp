#include "DatabaseConfig.hpp"

#include <limits>

#include <fmt/core.h>
#include <fmt/format.h>
#include <simdjson.h>

#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

namespace TwitchBot {

namespace json = simdjson;

auto ReadDatabaseConfig(std::u8string_view path) -> std::optional<DatabaseConfig>
{
  auto databaseJsonFilePath = std::string_view(reinterpret_cast<const char *>(path.data()), path.size());

  json::padded_string databaseJsonStr;
  auto error = json::padded_string::load(databaseJsonFilePath).get(databaseJsonStr);
  if (error) {
    fmt::print("Error: fail to load {}", databaseJsonFilePath);
    return std::nullopt;
  }

  json::ondemand::parser parser;
  json::ondemand::document databaseJson;
  error = parser.iterate(databaseJsonStr).get(databaseJson);
  if (error) {
    fmt::print("Error: {} file seem to be invalid json", databaseJsonFilePath);
    return std::nullopt;
  }

  std::string_view host;
  error = databaseJson["host"].get_string().get(host);
  if (error) {
    fmt::print("Error: host field is not a string");
    return std::nullopt;
  }

  int64_t portInt64;
  error = databaseJson["port"].get_int64().get(portInt64);
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

  std::string_view username;
  error = databaseJson["username"].get_string().get(username);
  if (error) {
    fmt::print("Error: username field is not a string");
    return std::nullopt;
  }

  std::string_view password;
  error = databaseJson["password"].get_string().get(password);
  if (error) {
    fmt::print("Error: password field is not a string");
    return std::nullopt;
  }

  std::string_view database;
  error = databaseJson["database"].get_string().get(database);
  if (error) {
    fmt::print("Error: database field is not a string");
    return std::nullopt;
  }

  fmt::print(
    "Database:\n\tHost: {}\n\tPort: {}\n\tUsername: {}\n\tUse password: {}\n\tDatabase: {}\n",
    host,
    port,
    username,
    "yes",
    database);


  auto encodeBuffer = std::u8string();
  auto decodeState  = ztd::text::make_decode_state(ztd::text::compat_utf8);
  auto encodeState  = ztd::text::make_encode_state(ztd::text::utf8);

  char32_t my_intermediate_buffer[256];
  std::span<char32_t> pivot(my_intermediate_buffer);

  auto transcode =
    [&decodeState, &encodeState, &pivot, &encodeBuffer](std::string_view input) -> const std::u8string & {
    encodeBuffer.clear();
    auto transcodeResult = ztd::text::transcode_into_raw(
      input,
      ztd::text::wide_execution,
      ztd::ranges::unbounded_view(std::back_inserter(encodeBuffer)),
      ztd::text::utf8,
      ztd::text::assume_valid_handler,
      ztd::text::assume_valid_handler,
      decodeState,
      encodeState,
      pivot);
    assert(transcodeResult.error_code == ztd::text::encoding_error::ok);
    return encodeBuffer;
  };

  return DatabaseConfig{
    .Host     = transcode(host),
    .Port     = port,
    .Username = transcode(username),
    .Password = transcode(password),
    .Database = transcode(database)
  };
}

}// namespace TwitchBot