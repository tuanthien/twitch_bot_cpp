#pragma once
#include <string>
#include "SystemMessageType.hpp"

namespace TwitchBot {
struct CommandResult
{
public:
  CommandResult(SystemMessageType type, const std::u8string &result);
  CommandResult(SystemMessageType type, std::u8string_view result);
  virtual SystemMessageType MessageType() const;
  virtual std::u8string Message() const;

protected:
  SystemMessageType type_;
  std::u8string result_;
};

struct Command
{
public:
  virtual auto Handle(std::u8string_view command) -> CommandResult = 0;
  virtual auto HelpString() -> std::u8string_view = 0;
};
}// namespace TwitchBot