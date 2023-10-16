#include "CppFormatMessage.hpp"
#include "MessageSerializer.hpp"
#include "Conversion.hpp"
#include <boost/json.hpp>
#include "Command.hpp"

namespace TwitchBot {

auto Serializer<CppFormatState::Success>::Serialize(int64_t refId, std::u8string_view formatted) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]            = json::value(static_cast<int64_t>(CommandMessageKind::CppFormatState));
  data["state"]          = json::value(static_cast<int64_t>(CppFormatState::Success));
  data["ref_id"]         = json::value(refId);
  data["formatted_code"] = json::value(to_string_view(formatted));
  obj["data"]            = std::move(data);
  return json::serialize(obj);
}

auto Serializer<CppFormatState::Error>::Serialize(int64_t refId, std::u8string_view formatted) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]           = json::value(static_cast<int64_t>(CommandMessageKind::CppFormatState));
  data["state"]         = json::value(static_cast<int64_t>(CppFormatState::Error));
  data["ref_id"]        = json::value(refId);
  data["error_message"] = json::value(to_string_view(formatted));
  obj["data"]           = std::move(data);
  return json::serialize(obj);
}


auto Serializer<CppFormatState::Formatting>::Serialize(int64_t refId) -> std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(static_cast<int64_t>(CommandMessageKind::CppFormatState));
  data["state"]  = json::value(static_cast<int64_t>(CppFormatState::Formatting));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}


}// namespace TwitchBot