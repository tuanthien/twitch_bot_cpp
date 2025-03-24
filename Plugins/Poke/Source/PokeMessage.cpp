#include "PokeMessage.hpp"
#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>
#include "Command.hpp"
#include "Poke.hpp"

namespace TwitchBot {

auto Serializer<PokeState::Success, PokeState>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]            = json::value(reinterpret_cast<const char *>(PLUGIN_ID.data()));
  data["state"]          = json::value(static_cast<int64_t>(PokeState::Success));
  data["ref_id"]         = json::value(refId);
  obj["data"]            = std::move(data);
  return json::serialize(obj);
}

auto Serializer<PokeState::Error, PokeState>::Serialize(int64_t refId, std::u8string_view message) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]           = json::value(reinterpret_cast<const char *>(PLUGIN_ID.data()));
  data["state"]         = json::value(static_cast<int64_t>(PokeState::Error));
  data["ref_id"]        = json::value(refId);
  data["error_message"] = json::value(to_string_view(message));
  obj["data"]           = std::move(data);
  return json::serialize(obj);
}


auto Serializer<PokeState::InProgress, PokeState>::Serialize(int64_t refId, std::u8string_view from) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(reinterpret_cast<const char *>(PLUGIN_ID.data()));
  data["state"]  = json::value(static_cast<int64_t>(PokeState::InProgress));
  data["ref_id"] = json::value(refId);
  data["from"]   = json::value(to_string_view(from));
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}

auto Serializer<PokeState::Timeout, PokeState>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(reinterpret_cast<const char *>(PLUGIN_ID.data()));
  data["state"]  = json::value(static_cast<int64_t>(PokeState::Timeout));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}

auto Serializer<PokeState::Reject, PokeState>::Serialize(int64_t refId, uint64_t nextInMillis) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(reinterpret_cast<const char *>(PLUGIN_ID.data()));
  data["state"]  = json::value(static_cast<int64_t>(PokeState::Reject));
  data["ref_id"] = json::value(refId);
  data["next_in_millis"] = json::value(nextInMillis);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}


}// namespace TwitchBot