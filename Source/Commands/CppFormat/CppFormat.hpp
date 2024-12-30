#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <memory>

#include "Command.hpp"
#include "TwitchBotConfig.hpp"
#include <boost/asio/awaitable.hpp>

namespace TwitchBot {

struct Broadcaster;

struct CppFormat : public Command
{
public:
  CppFormat(const std::shared_ptr<Broadcaster>& broadcaster, const BotCommandCppConfig& config);
  auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG>& command, void *userData) -> boost::asio::awaitable<std::optional<CommandResult>> override;
  auto HelpString() -> std::u8string_view override;
private:
  std::shared_ptr<Broadcaster> broadcaster_;
  const BotCommandCppConfig* config_;
};

}