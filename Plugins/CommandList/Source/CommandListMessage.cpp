#include "CommandListMessage.hpp"
#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>
#include "Command.hpp"
#include "CommandList.hpp"

namespace TwitchBot {

auto Serializer<CommandListState::Success, CommandListState>::Serialize(int64_t refId, std::u8string_view commands)
  -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]      = json::value(reinterpret_cast<const char*>(PLUGIN_ID.data()));
  data["state"]    = json::value(static_cast<int64_t>(CommandListState::Success));
  data["ref_id"]   = json::value(refId);
  data["commands"] = json::value(to_string_view(commands));
  obj["data"]      = std::move(data);
  return json::serialize(obj);
}

auto Serializer<CommandListState::Error, CommandListState>::Serialize(int64_t refId, std::u8string_view message)
  -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]           = json::value(reinterpret_cast<const char*>(PLUGIN_ID.data()));
  data["state"]         = json::value(static_cast<int64_t>(CommandListState::Error));
  data["ref_id"]        = json::value(refId);
  data["error_message"] = json::value(to_string_view(message));
  obj["data"]           = std::move(data);
  return json::serialize(obj);
}


auto Serializer<CommandListState::Querying, CommandListState>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(reinterpret_cast<const char*>(PLUGIN_ID.data()));
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
  obj["kind"]    = json::value(reinterpret_cast<const char*>(PLUGIN_ID.data()));
  data["state"]  = json::value(static_cast<int64_t>(CommandListState::Timeout));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}

}// namespace TwitchBot