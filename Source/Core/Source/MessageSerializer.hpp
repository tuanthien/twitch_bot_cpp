#pragma once

#include "TwitchIRCParser.hpp"
#include <string>
#include <boost/json.hpp>
#include "Conversion.hpp"
#include "Command.hpp"

namespace TwitchBot {

// @TODO move this some where
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};


inline auto Serialize(
  const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &parameters, int64_t serializeId, bool modified = false)
  -> std::string
{
  namespace json = boost::json;
  json::object obj;

  obj["kind"] = json::value("00000000-0000-0000-0000-000000000000");

  json::object data;
  json::array parts;
  for (const auto &part : parameters.Parts) {
    parts.push_back(std::visit(
      overloaded{
        [](const IRC::TextPart &text) {
          json::object jsonPart;
          jsonPart["type"]  = json::value("text");
          jsonPart["value"] = json::value(to_string_view(text.Value));
          return jsonPart;
        },
        [](const IRC::EmotePart &emote) {
          json::object jsonPart;
          jsonPart["type"]  = json::value("emote");
          jsonPart["value"] = json::value(to_string_view(emote.Value));
          return jsonPart;
        }},
      part));
  }
  data["id"]           = json::value(static_cast<int64_t>(serializeId));
  data["display_name"] = json::value(to_string_view(parameters.DisplayName));
  data["parts"]        = std::move(parts);
  data["modified"]     = json::value(modified);
  obj["data"]          = std::move(data);

  return json::serialize(obj);
}

}// namespace TwitchBot