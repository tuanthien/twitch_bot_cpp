#pragma once
#include "Command.hpp"
#include "PluginInfo.hpp"
#include <boost/asio.hpp>
#include <string_view>

namespace TwitchBot {
struct PluginModule
{
  using FPN_GetInfo    = PluginInfo *(*)();
  using FPN_DatabaseUp = boost::asio::awaitable<bool> (*)(boost::asio::io_context &, std::u8string_view, ConnectParams);
  using FPN_DatabaseDown = boost::asio::awaitable<bool> (*)(boost::asio::io_context &, std::u8string_view);
  using FPN_CreateCommandFactory =
    std::shared_ptr<CommandFactory> (*)(std::u8string_view, boost::asio::io_context &, const ConnectParams &);
  using FPN_Configure = boost::asio::awaitable<bool> (*)(
    boost::asio::io_context &, const std::vector<std::pair<std::u8string_view, FieldType>> &);

  PluginModule(
    HMODULE module,
    FPN_GetInfo infoFunc,
    FPN_DatabaseUp databaseUpFunc,
    FPN_DatabaseDown databaseDownFunc,
    FPN_CreateCommandFactory createCommandFactoryFunc,
    FPN_Configure configureFunc)
    : module_(module)
    , infoFunc_(infoFunc)
    , databaseUpFunc_(databaseUpFunc)
    , databaseDownFunc_(databaseDownFunc)
    , createCommandFactoryFunc_(createCommandFactoryFunc)
    , configureFunc_(configureFunc)
  {}

  auto
    Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries) const
    -> boost::asio::awaitable<bool>
  {

    co_return false;
  }

  auto Id() const -> std::u8string_view { return infoFunc_()->Id(); }
  auto Name() const -> std::u8string_view { return infoFunc_()->Name(); }
  auto Version() const -> std::u8string_view { return infoFunc_()->Version(); }
  auto ConfigDescriptor() const -> std::span<const std::pair<std::u8string_view, FieldDescription>>
  {
    return infoFunc_()->ConfigDescriptor();
  }

  inline auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
    -> boost::asio::awaitable<bool>
  {
    return databaseUpFunc_(ioc, oldVersion, connectDb);
  }

  inline auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion) -> boost::asio::awaitable<bool>
  {
    return databaseDownFunc_(ioc, oldVersion);
  }

  inline auto
    CreateCommandFactory(std::u8string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
      -> std::shared_ptr<CommandFactory>
  {
    return createCommandFactoryFunc_(command, ioc, connectDb);
  }

  constexpr static std::string_view DatabaseUpSignature =
    "?DatabaseUp@TwitchBot@@YA?AV?$awaitable@_NVany_io_executor@asio@boost@@@asio@boost@@AEAVio_context@34@V?$basic_"
    "string_view@_QU?$char_traits@_Q@std@@@std@@UConnectParams@1@@Z";
  constexpr static std::string_view DatabaseDownSignature =
    "?DatabaseDown@TwitchBot@@YA?AV?$awaitable@_NVany_io_executor@asio@boost@@@asio@boost@@AEAVio_context@34@V?$basic_"
    "string_view@_QU?$char_traits@_Q@std@@@std@@@Z";
  constexpr static std::string_view GetInfoSignature = "?GetInfo@TwitchBot@@YAPEAUPluginInfo@1@XZ";
  constexpr static std::string_view CreateCommandFactorySignature =
    "?CreateCommandFactory@TwitchBot@@YA?AV?$shared_ptr@UCommandFactory@TwitchBot@@@std@@V?$basic_string_view@DU?$char_"
    "traits@D@std@@@3@AEAVio_context@asio@boost@@AEBUConnectParams@1@@Z";
  constexpr static std::string_view ConfigureSignature =
    "?Configure@TwitchBot@@YA?AV?$awaitable@_NVany_io_executor@asio@boost@@@asio@boost@@AEAVio_context@34@AEBV?$vector@"
    "U?$pair@V?$basic_string_view@_QU?$char_traits@_Q@std@@@std@@V?$variant@_NHIV?$basic_string@DU?$char_traits@D@std@@"
    "V?$allocator@D@2@@std@@@2@@std@@V?$allocator@U?$pair@V?$basic_string_view@_QU?$char_traits@_Q@std@@@std@@V?$"
    "variant@_NHIV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@2@@std@@@Z";

private:
  HMODULE module_;
  FPN_GetInfo infoFunc_;
  FPN_DatabaseUp databaseUpFunc_;
  FPN_DatabaseDown databaseDownFunc_;
  FPN_CreateCommandFactory createCommandFactoryFunc_;
  FPN_Configure configureFunc_;
};
}// namespace TwitchBot