#include "Command.hpp"


namespace TwitchBot {

CommandResult::CommandResult(SystemMessageType type, std::u8string_view result)
  : type_(type)
  , result_(result)
{}

CommandResult::CommandResult(SystemMessageType type, const std::u8string &result)
  : type_(type)
  , result_(result)
{}

auto CommandResult::MessageType() const -> SystemMessageType
{
  return type_;
}

auto CommandResult::Message() const -> std::u8string
{
  return result_;
}

}// namespace TwitchBot