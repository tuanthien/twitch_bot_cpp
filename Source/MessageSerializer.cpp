#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>
#include "Command.hpp"

namespace TwitchBot {

auto Serialize(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &parameters, int64_t serializeId, bool modified) -> std::string
{
  namespace json = boost::json;
  json::object obj;

  obj["kind"] = json::value(static_cast<int64_t>(CommandMessageKind::Generic));

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
  data["id"] = json::value(static_cast<int64_t>(serializeId));
  data["display_name"] = json::value(to_string_view(parameters.DisplayName));
  data["parts"] = std::move(parts);
  data["modified"] = json::value(modified);
  obj["data"]   = std::move(data);

  return json::serialize(obj);
}

}// namespace TwitchBot