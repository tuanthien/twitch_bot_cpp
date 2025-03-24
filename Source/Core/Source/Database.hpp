#pragma once
#include <string_view>

namespace TwitchBot {

struct ConnectParams
{
  std::u8string_view Host;
  std::uint16_t Port{3306U};
  std::u8string_view Username;
  std::u8string_view Password;
  std::u8string_view Database;
  std::uint16_t ConnectionCollation{45};
  bool SSL{true};
  bool MultiQueries{false};
};

}// namespace TwitchBot