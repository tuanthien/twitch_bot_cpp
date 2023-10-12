#include <string>
#include <string_view>
#include <cstring>

namespace TwitchBot {

inline auto to_u8string(std::string_view view) -> std::u8string
{
  std::u8string u8str;
  u8str.resize(view.size());// @TODO need C++23 resize_and_overwrite
  memcpy(u8str.data(), view.data(), u8str.size());
  return u8str;
}
inline auto to_string_view(const std::u8string &str) -> std::string_view
{
  return std::string_view(reinterpret_cast<const char *>(str.data()), str.size());
}

inline auto to_string_view(const std::u8string_view &str) -> std::string_view
{
  return std::string_view(reinterpret_cast<const char *>(str.data()), str.size());
}

}// namespace TwitchBot