#pragma once
#include <string_view>
#include "Windows/PlatformInclude.hpp"
#include "Conversion.hpp"
#include "Command.hpp"
#include "PluginInfo.hpp"
#include "PluginModule.hpp"
#include <fmt/format.h>
#include <boost/asio.hpp>

#include <boost/mysql/any_address.hpp>
#include <boost/mysql/any_connection.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/results.hpp>
#include <boost/mysql/with_params.hpp>
#include <iostream>

#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

namespace mysql = boost::mysql;

namespace TwitchBot {
struct PluginRegistry
{
  PluginRegistry(boost::asio::io_context &ioc, mysql::any_connection &dbcon)
    : ioc_(std::addressof(ioc))
    , dbcon_(std::addressof(dbcon))
  {}

  auto LoadPlugin(boost::asio::io_context &ioc, std::u8string_view moduleFile, const ConnectParams &connectDb)
    -> boost::asio::awaitable<std::shared_ptr<PluginModule>>
  {
    auto encodeBuffer = std::wstring();
    auto decodeState  = ztd::text::make_decode_state(ztd::text::utf8);
    auto encodeState  = ztd::text::make_encode_state(ztd::text::wide_execution);
    // @TODO need to do something about this buffer
    char32_t intermediateBuffer[256];
    std::span<char32_t> pivot(intermediateBuffer);
    auto transcodeResult = ztd::text::transcode_into_raw(
      moduleFile,
      ztd::text::utf8,
      ztd::ranges::unbounded_view(std::back_inserter(encodeBuffer)),
      ztd::text::wide_execution,
      ztd::text::replacement_handler,
      ztd::text::replacement_handler,
      decodeState,
      encodeState,
      pivot);

    HMODULE nativeModule = LoadLibraryExW(encodeBuffer.c_str(), nullptr, 0);

    auto infoFnc = reinterpret_cast<typename PluginModule::FPN_GetInfo>(
      GetProcAddress(nativeModule, PluginModule::GetInfoSignature.data()));
    assert(infoFnc);

    auto databaseUpFnc = reinterpret_cast<typename PluginModule::FPN_DatabaseUp>(
      GetProcAddress(nativeModule, PluginModule::DatabaseUpSignature.data()));
    assert(databaseUpFnc);

    auto databaseDownFnc = reinterpret_cast<typename PluginModule::FPN_DatabaseDown>(
      GetProcAddress(nativeModule, PluginModule::DatabaseDownSignature.data()));
    assert(databaseDownFnc);

    auto createCommandFnc = reinterpret_cast<typename PluginModule::FPN_CreateCommandFactory>(
      GetProcAddress(nativeModule, PluginModule::CreateCommandFactorySignature.data()));
    assert(createCommandFnc);

    auto configureFnc = reinterpret_cast<typename PluginModule::FPN_Configure>(
      GetProcAddress(nativeModule, PluginModule::ConfigureSignature.data()));
    // auto x = GetLastError();
    assert(configureFnc);

    auto plugin = std::make_shared<PluginModule>(
      nativeModule, infoFnc, databaseUpFnc, databaseDownFnc, createCommandFnc, configureFnc);

    bool regiserdbInfo = co_await Register(ioc, *plugin, connectDb);
    if (not regiserdbInfo) {
      co_return nullptr;
    }
    modules_.emplace(plugin->Id(), plugin);
    co_return plugin;
  }

  auto UnloadPlugin() -> boost::asio::awaitable<bool>
  {
    // call command module unload -> cancel all pending operation in asio context -> wait for synchronization -> unload
    // dll
  }

  auto Register(boost::asio::io_context &ioc, PluginModule &module, const ConnectParams &connectDb)
    -> boost::asio::awaitable<bool>
  {
    auto queryCommand = dbcon_->prepare_statement("SELECT id, name, version, enabled FROM plugins WHERE id=?");
    mysql::results dbInfo;
    auto moduleId = to_string_view(module.Id());
    auto [ec] = co_await dbcon_->async_execute(queryCommand.bind(moduleId), dbInfo, boost::asio::as_tuple);
    if (ec != boost::system::errc::success) co_return false;

    if (dbInfo.rows().empty()) {
      mysql::results insertResult;
      auto [ec] = co_await dbcon_->async_execute(
        mysql::with_params(
          "INSERT INTO plugins(id, name, version, enabled) VALUES ({}, {}, {}, TRUE)",
          to_string_view(module.Id()),
          to_string_view(module.Name()),
          to_string_view(module.Version())),
        insertResult, boost::asio::as_tuple);
 
      if (ec != boost::system::errc::success) co_return false;

      if (not insertResult.rows().empty()) co_return false;

      bool upSucceeded = co_await module.DatabaseUp(ioc, u8"", connectDb);
      co_return upSucceeded;

    } else {
      int64_t enabled = dbInfo.rows().at(0).at(3).get_int64();
      if (enabled) {
        std::string_view version = dbInfo.rows().at(0).at(2).as_string();
        if (version == to_string_view(module.Version())) {
          // if already exists in database and version the same then just return success
          co_return true;
        } else {
          // @TODO version compare, assume loaded module version > registered version

          // if already exists in database but the version not the same then return db_user and run command module
          // database up function
        }

      } else {
        // if not enabled then return failure
        co_return false;
      }
    }

    co_return true;
  }

  auto Unregister(boost::asio::io_context &ioc, const PluginInfo *info, void *module) -> boost::asio::awaitable<bool>
  {
    co_return true;
    // assume existed and same version
    // run command module database down function
    // remove db_user
    // remove from registry
  }

private:
  boost::asio::io_context *ioc_;
  mysql::any_connection *dbcon_;
  std::unordered_map<std::u8string, std::shared_ptr<PluginModule>> modules_;
};
}// namespace TwitchBot