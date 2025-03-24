#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <memory>

#include "Command.hpp"
#include <boost/asio/awaitable.hpp>
#include "CommandListExport.hpp"
#include "Database.hpp"
#include "PluginInfo.hpp"

namespace TwitchBot {

struct Broadcaster;

struct CommandList : public CommandHandler
{
public:
  COMMANDLIST_API CommandList(const std::shared_ptr<Broadcaster> &broadcaster);
  COMMANDLIST_API auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
    -> boost::asio::awaitable<std::optional<CommandResult>> override;

private:
  std::shared_ptr<Broadcaster> broadcaster_;
};

constexpr inline std::u8string_view PLUGIN_ID = u8"1c66ecd2-663c-4e04-975f-a6593432e53c";

struct CommandListInfo : public PluginInfo
{
  auto Id() const -> std::u8string_view override { return PLUGIN_ID; }
  auto Name() const -> std::u8string_view override { return u8"CommandList"; }
  auto Version() const -> std::u8string_view { return u8"0.1"; };
  auto BotVersion() const -> std::u8string_view { return u8"0.1"; };
  auto Help() const -> std::u8string_view override { return u8"list all commands available"; }
  auto Description() const -> std::u8string_view override { return u8"list all commands available"; }
  auto ConfigDescriptor() const -> std::span<const std::pair<std::u8string_view, FieldDescription>> override
  {
    constexpr static std::array<std::pair<std::u8string_view, FieldDescription>, 0> descriptor = {};
    return std::span<const std::pair<std::u8string_view, FieldDescription>, std::dynamic_extent>(
      descriptor.cbegin(), descriptor.cend());
  }
};

struct CommandListFactory : public CommandFactory
{
  auto Create(const std::shared_ptr<Broadcaster> &broadcaster) -> boost::asio::awaitable<std::shared_ptr<CommandHandler>> override;
};

COMMANDLIST_API auto CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams& connectDb) -> std::shared_ptr<CommandFactory>;
COMMANDLIST_API auto GetInfo() -> PluginInfo *;
COMMANDLIST_API auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>;
COMMANDLIST_API auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion)
  -> boost::asio::awaitable<bool>;
COMMANDLIST_API auto
  Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
    -> boost::asio::awaitable<bool>;
}// namespace TwitchBot