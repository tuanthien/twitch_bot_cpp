#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>

namespace TwitchBot {

auto Serialize(const CommandParameters<IRCCommand::PRIVMSG> &parameters) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  obj["id"] = json::value(static_cast<int64_t>(IRCCommand::PRIVMSG));

  json::object message;
  message["display_name"] = json::value(to_string_view(parameters.DisplayName));

  json::array parts;
  for (const auto &part : parameters.Parts) {
    parts.push_back(std::visit(
      overloaded{
        [](const TextPart &text) {
          json::object jsonPart;
          jsonPart["type"]  = json::value("text");
          jsonPart["value"] = json::value(to_string_view(text.Value));
          return jsonPart;
        },
        [](const EmotePart &emote) {
          json::object jsonPart;
          jsonPart["type"]  = json::value("emote");
          jsonPart["value"] = json::value(to_string_view(emote.Value));
          return jsonPart;
        }},
      part));
  }

  message["parts"] = std::move(parts);
  obj["message"]   = std::move(message);

  return json::serialize(obj);
}

}// namespace TwitchBot