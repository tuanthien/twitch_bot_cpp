#include "CommandListMessage.hpp"
#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>
#include "Command.hpp"

namespace TwitchBot {

auto Serializer<CommandListState::Success, CommandListState>::Serialize(int64_t refId, std::u8string_view commandList)
  -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]          = json::value(static_cast<int64_t>(CommandMessageKind::CommandListState));
  data["state"]        = json::value(static_cast<int64_t>(CommandListState::Success));
  data["ref_id"]       = json::value(refId);
  data["command_list"] = json::value(to_string_view(commandList));
  obj["data"]          = std::move(data);
  return json::serialize(obj);
}

auto Serializer<CommandListState::Error, CommandListState>::Serialize(int64_t refId, std::string_view message)
  -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]           = json::value(static_cast<int64_t>(CommandMessageKind::CommandListState));
  data["state"]         = json::value(static_cast<int64_t>(CommandListState::Error));
  data["ref_id"]        = json::value(refId);
  data["error_message"] = json::value(message);
  obj["data"]           = std::move(data);
  return json::serialize(obj);
}


auto Serializer<CommandListState::Querying, CommandListState>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(static_cast<int64_t>(CommandMessageKind::CommandListState));
  data["state"]  = json::value(static_cast<int64_t>(CommandListState::Querying));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}

auto Serializer<CommandListState::Timeout, CommandListState>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(static_cast<int64_t>(CommandMessageKind::CommandListState));
  data["state"]  = json::value(static_cast<int64_t>(CommandListState::Timeout));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}

}// namespace TwitchBot