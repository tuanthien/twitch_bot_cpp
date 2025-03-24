#pragma once
#include <string>
#include <optional>
#include "SystemMessageType.hpp"
#include "TwitchIRCParser.hpp"
#include "Broadcaster.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

namespace TwitchBot {

struct CommandResult
{
public:
  CommandResult(SystemMessageType type, const std::u8string &result)
    : type_(type)
    , result_(result)
  {}
  CommandResult(SystemMessageType type, std::u8string_view result)
    : type_(type)
    , result_(result)
  {}
  virtual SystemMessageType MessageType() const { return type_; }
  virtual std::u8string Message() const { return result_; }


protected:
  SystemMessageType type_;
  std::u8string result_;
};

struct CommandHandler
{
public:
  virtual auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
    -> boost::asio::awaitable<std::optional<CommandResult>> = 0;
};

struct CommandFactory
{
  virtual auto Create(const std::shared_ptr<Broadcaster> &broadcaster) -> boost::asio::awaitable<std::shared_ptr<CommandHandler>> = 0;
};

}// namespace TwitchBot