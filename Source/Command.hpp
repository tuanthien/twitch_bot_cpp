#pragma once
#include <string>
#include <optional>
#include "SystemMessageType.hpp"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include "TwitchIRCParser.hpp"

namespace TwitchBot {

enum struct CommandMessageKind {
  Generic,
  CppFormatState
};

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
  virtual auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG>& command, void* userData) -> boost::asio::awaitable<std::optional<CommandResult>> = 0;
  virtual auto HelpString() -> std::u8string_view = 0;
};
}// namespace TwitchBot