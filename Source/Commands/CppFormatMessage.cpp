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


static auto serialize(CppFormatState state, int64_t refId)->std::string
{
  namespace json = boost::json;
  json::object obj;
  json::object data;
  obj["kind"]    = json::value(static_cast<int64_t>(CommandMessageKind::CppFormatState));
  data["state"]  = json::value(static_cast<int64_t>(state));
  data["ref_id"] = json::value(refId);
  obj["data"]    = std::move(data);
  return json::serialize(obj);
}


auto Serializer<CppFormatState::WritingFile>::Serialize(int64_t refId) -> std::string
{
  return serialize(CppFormatState::WritingFile, refId);
}

auto Serializer<CppFormatState::LaunchingFormatter>::Serialize(int64_t refId) -> std::string
{
  return serialize(CppFormatState::LaunchingFormatter, refId);
}

}// namespace TwitchBot