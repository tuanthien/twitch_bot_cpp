#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <memory>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include "Command.hpp"
#include "CppFormatExport.hpp"
#include "Database.hpp"
#include "PluginInfo.hpp"

#include <boost/mysql/any_connection.hpp>


namespace TwitchBot {

namespace mysql = boost::mysql;

struct Broadcaster;

struct CppFormatConfig
{
  uint64_t Id;
  std::u8string Version;
  std::u8string ClangFormatPath;
  std::u8string ClangFormatConfigPath;
  uint64_t TimeoutInMillis;
};


struct CppFormat : public CommandHandler
{
public:
  CPPFORMAT_API CppFormat(CppFormatConfig &&config, const std::shared_ptr<Broadcaster> &broadcaster);
  CPPFORMAT_API auto Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
    -> boost::asio::awaitable<std::optional<CommandResult>> override;

private:
  std::shared_ptr<Broadcaster> broadcaster_;
  CppFormatConfig config_;
};

constexpr inline std::u8string_view PLUGIN_ID = u8"ee50b83b-9182-4e31-b33f-a6b94722cc85";

struct CppFormatInfo : public PluginInfo
{
  auto Id() const -> std::u8string_view override { return u8"ee50b83b-9182-4e31-b33f-a6b94722cc85"; }
  auto Name() const -> std::u8string_view override { return u8"CppFormat"; }
  auto Version() const -> std::u8string_view { return u8"0.1"; };
  auto BotVersion() const -> std::u8string_view { return u8"0.1"; };
  auto Help() const -> std::u8string_view override { return u8"input C++ code for clang-format"; }
  auto Description() const -> std::u8string_view override { return u8"input C++ code for clang-format"; }
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

struct CppFormatFactory : public CommandFactory
{
  CppFormatFactory(boost::asio::io_context &ioc, const ConnectParams &connectDb)
    : connected_(false)
    , conn_(ioc)
    , conParams_(connectDb)
  {}

  auto Create(const std::shared_ptr<Broadcaster> &broadcaster)
    -> boost::asio::awaitable<std::shared_ptr<CommandHandler>> override;

  bool connected_;
  mysql::any_connection conn_;
  ConnectParams conParams_;
};

CPPFORMAT_API auto
  CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
    -> std::shared_ptr<CommandFactory>;
CPPFORMAT_API auto GetInfo() -> PluginInfo *;
CPPFORMAT_API auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>;
CPPFORMAT_API auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion)
  -> boost::asio::awaitable<bool>;
CPPFORMAT_API auto
  Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
    -> boost::asio::awaitable<bool>;
}// namespace TwitchBot