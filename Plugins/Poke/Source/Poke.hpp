#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <chrono>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include "Command.hpp"
#include "PokeExport.hpp"
#include "Database.hpp"
#include "PluginInfo.hpp"


namespace TwitchBot {

struct Broadcaster;

struct PokeConfig
{
  std::chrono::seconds Cooldown;
};


struct Poke : public CommandHandler
{
public:
  POKE_API Poke(PokeConfig &&config, const std::shared_ptr<Broadcaster> &broadcaster);
  POKE_API auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
    -> boost::asio::awaitable<std::optional<CommandResult>> override;

private:
  std::shared_ptr<Broadcaster> broadcaster_;
  std::chrono::utc_clock::time_point lastPoke_;
  PokeConfig config_;
};

constexpr inline std::u8string_view PLUGIN_ID = u8"c858e29c-c320-4de6-a227-013d8f7a90f0";

struct PokeInfo : public PluginInfo
{
  auto Id() const -> std::u8string_view override { return PLUGIN_ID; }
  auto Name() const -> std::u8string_view override { return u8"Poke"; }
  auto Version() const -> std::u8string_view { return u8"0.1"; };
  auto BotVersion() const -> std::u8string_view { return u8"0.1"; };
  auto Help() const -> std::u8string_view override { return u8"poke broadcaster to get attention"; }
  auto Description() const -> std::u8string_view override { return u8"poke broadcaster to get attention"; }
  auto ConfigDescriptor() const -> std::span<const std::pair<std::u8string_view, FieldDescription>> override
  {
    constexpr static std::array<std::pair<std::u8string_view, FieldDescription>, 5> descriptor = {
      std::pair<std::u8string_view, FieldDescription>{u8"version", FieldDescription::String},
      {u8"clang_format_path", FieldDescription::String},
      {u8"clang_format_config_path", FieldDescription::String},
      {u8"cpp_temp_path", FieldDescription::String},
      {u8"timeout_in_millis", FieldDescription::UInt},
    };
    return std::span<const std::pair<std::u8string_view, FieldDescription>, std::dynamic_extent>(
      descriptor.cbegin(), descriptor.cend());
  }
};

struct PokeFactory : public CommandFactory
{
  PokeFactory(boost::asio::io_context &ioc)
  {}

  auto Create(const std::shared_ptr<Broadcaster> &broadcaster)
    -> boost::asio::awaitable<std::shared_ptr<CommandHandler>> override;
};

POKE_API auto
  CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
    -> std::shared_ptr<CommandFactory>;
POKE_API auto GetInfo() -> PluginInfo *;
POKE_API auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>;
POKE_API auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion) -> boost::asio::awaitable<bool>;
POKE_API auto
  Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
    -> boost::asio::awaitable<bool>;
}// namespace TwitchBot