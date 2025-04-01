#include "CppFormat.hpp"

#include <boost/asio.hpp>
#include "boost/asio/experimental/awaitable_operators.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/process/v2.hpp>
#include "Conversion.hpp"
#include <fmt/format.h>
#include "Broadcaster.hpp"
#include "CppFormatMessage.hpp"
#include "MessageSerializer.hpp"
#include <fstream>
#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

#include <boost/mysql/any_address.hpp>
#include <boost/mysql/any_connection.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/results.hpp>
#include <boost/mysql/with_params.hpp>
#include <filesystem>
#include <tuple>


namespace TwitchBot {

namespace proc  = boost::process::v2;
namespace asio  = boost::asio;
namespace mysql = boost::mysql;

auto CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
  -> std::shared_ptr<CommandFactory>
{
  if (command == "cpp") {
    return std::make_shared<CppFormatFactory>(ioc, connectDb);
  } else {
    return nullptr;
  }
}

auto GetInfo() -> PluginInfo *
{
  static auto registry = CppFormatInfo();
  return static_cast<PluginInfo *>(&registry);
}

auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>
{
  if (oldVersion == GetInfo()->Version()) co_return true;

  mysql::any_connection conn(ioc);
  mysql::connect_params params;
  params.server_address.emplace_host_and_port(
    std::string(reinterpret_cast<const char *>(connectDb.Host.data())), connectDb.Port);
  params.username = reinterpret_cast<const char *>(connectDb.Username.data());
  params.password = reinterpret_cast<const char *>(connectDb.Password.data());
  params.database = reinterpret_cast<const char *>(connectDb.Database.data());

  // Connect to the server
  auto mysqlConnectResult = co_await conn.async_connect(params, asio::as_tuple);

  if (std::get<0>(mysqlConnectResult) != boost::system::errc::success) co_return false;

  mysql::results createSetting;
  auto ec = std::get<0>(co_await conn.async_execute(
    R"sql(
      CREATE TABLE plugin_cppformat (
        id INT auto_increment NOT NULL,
        version varchar(32) NOT NULL,
        clang_format_path TEXT NULL,
        clang_format_config_path TEXT NULL,
        cpp_temp_path TEXT NULL,
        timeout_in_millis INT UNSIGNED NULL,
        CONSTRAINT plugin_cppformat_pk PRIMARY KEY (id)
      )
      ENGINE=InnoDB
      DEFAULT CHARSET=utf8mb4
      COLLATE=utf8mb4_unicode_ci;
    )sql",
    createSetting,
    asio::as_tuple));

  assert(ec);

  // mysql::results insertSetting;
  // ec = std::get<0>(co_await conn.async_execute(
  //   mysql::with_params(
  //     R"sql(
  //     INSERT INTO plugin_cppformat (version, clang_format_path, clang_format_config_path, cpp_temp_path,
  //     timeout_in_millis) VALUES ({}, {}, {}, {}, {})
  //   )sql",
  //     reinterpret_cast<const char *>(GetInfo()->Version().data()),
  //     "1",
  //     "2",
  //     "3",
  //     1000),
  //     insertSetting,
  //   asio::as_tuple));

  // assert(ec);

  co_return true;
}

auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion) -> boost::asio::awaitable<bool>
{
  // @TODO
  // if(oldVersion < GetInfo()->Version()) {
  // version 0.1 is the first version so can't be down to nothing. maybe implement an uninstall function
  // } else {
  // invalid oldVersion
  // }
  co_return false;
}

auto Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
  -> boost::asio::awaitable<bool>
{

  co_return false;
}


auto CppFormatFactory::Create(const std::shared_ptr<Broadcaster> &broadcaster)
  -> boost::asio::awaitable<std::shared_ptr<CommandHandler>>
{
  if (not connected_) {
    auto params = mysql::connect_params();
    params.server_address.emplace_host_and_port(
      std::string(reinterpret_cast<const char *>(conParams_.Host.data())), conParams_.Port);
    params.username = reinterpret_cast<const char *>(conParams_.Username.data());
    params.password = reinterpret_cast<const char *>(conParams_.Password.data());
    params.database = reinterpret_cast<const char *>(conParams_.Database.data());

    auto [ec] = co_await conn_.async_connect(params, asio::as_tuple);
    if (ec != boost::system::errc::success) co_return nullptr;
    connected_ = true;
  }

  mysql::results config;
  auto [ec] = co_await conn_.async_execute(
    R"sql(
      SELECT id, version, clang_format_path, clang_format_config_path, timeout_in_millis 
      FROM plugin_cppformat 
      ORDER BY id
      DESC LIMIT 1;
    )sql",
    config,
    asio::as_tuple);

  if (config.rows().empty()) {
    co_return nullptr;
  }

  auto id                       = config.rows().at(0).at(0).get_uint64();
  auto version                  = config.rows().at(0).at(1).get_string();
  auto clang_format_path        = config.rows().at(0).at(2).get_string();
  auto clang_format_config_path = config.rows().at(0).at(3).get_string();
  auto timeout_in_millis        = config.rows().at(0).at(4).get_uint64();


  auto encodeBuffer = std::u8string();
  auto decodeState  = ztd::text::make_decode_state(ztd::text::compat_utf8);
  auto encodeState  = ztd::text::make_encode_state(ztd::text::utf8);

  char32_t my_intermediate_buffer[8'192];
  std::span<char32_t> pivot(my_intermediate_buffer);

  auto transcode =
    [&decodeState, &encodeState, &pivot, &encodeBuffer](std::string_view input) -> const std::u8string & {
    encodeBuffer.clear();
    auto transcodeResult = ztd::text::transcode_into_raw(
      input,
      ztd::text::wide_execution,
      ztd::ranges::unbounded_view(std::back_inserter(encodeBuffer)),
      ztd::text::utf8,
      ztd::text::assume_valid_handler,
      ztd::text::assume_valid_handler,
      decodeState,
      encodeState,
      pivot);

    assert(transcodeResult.error_code == ztd::text::encoding_error::ok);

    return encodeBuffer;
  };

  co_return std::make_shared<CppFormat>(
    CppFormatConfig{
      .Id                    = id,
      .Version               = transcode(version),
      .ClangFormatPath       = transcode(clang_format_path),
      .ClangFormatConfigPath = transcode(clang_format_config_path),
      .TimeoutInMillis       = timeout_in_millis},
    broadcaster);
}

CppFormat::CppFormat(CppFormatConfig &&config, const std::shared_ptr<Broadcaster> &broadcaster)
  : broadcaster_(broadcaster)
  , config_(std::move(config))
{}

static auto write_to_clang_format(asio::writable_pipe &formatIn, std::u8string_view code)
  -> asio::awaitable<std::tuple<boost::system::error_code, size_t>>
{
  constexpr static char8_t eofChar = 0;
  const auto eof                   = std::u8string_view(&eofChar, 1);
  auto [writeErr, writeSize]       = co_await asio::async_write(
    formatIn, asio::const_buffer(code.data(), code.size()), asio::as_tuple(asio::use_awaitable));
  assert(writeSize == code.size());

  if (writeErr != boost::system::errc::success) {
    fmt::println("write_to_clang_format: {}", writeErr.what());
    co_return std::tuple<boost::system::error_code, size_t>(writeErr, size_t{0});
  }

  formatIn.close();

  co_return std::tuple<boost::system::error_code, size_t>(boost::system::error_code(), writeSize);
}

auto CppFormat::Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
  -> asio::awaitable<std::optional<CommandResult>>
{
  auto &[displayName, msgParts] = command;

  auto commandIdx = std::bit_cast<intptr_t>(userData);
  broadcaster_->Send(Serialize(command, commandIdx, true));
  broadcaster_->Send(Serializer<CppFormatState::Formatting>{}.Serialize(commandIdx));

  std::u8string_view chatText = std::get<IRC::TextPart>(msgParts[0]).Value;
  chatText.remove_prefix(5);

  auto styleFile =
    fmt::format("--style=file:{}", reinterpret_cast<const char *>(config_.ClangFormatConfigPath.c_str()));

  asio::steady_timer timeout{co_await asio::this_coro::executor, std::chrono::milliseconds(config_.TimeoutInMillis)};
  asio::cancellation_signal sig;
  std::u8string buffer;
  asio::readable_pipe formatOut(co_await asio::this_coro::executor);
  asio::writable_pipe formatIn(co_await asio::this_coro::executor);

  proc::async_execute(
    proc::process(
      co_await asio::this_coro::executor,
      reinterpret_cast<const char *>(config_.ClangFormatPath.c_str()),
      {styleFile, "--assume-filename=code.cpp"},
      proc::process_stdio{formatIn, formatOut, {}}),
    asio::bind_cancellation_slot(sig.slot(), [&](boost::system::error_code ec, int exit_code) {
      if (ec or exit_code != 0) { /* @TODO log error? */
      }
      timeout.cancel();
    }));

  using namespace boost::asio::experimental::awaitable_operators;
  auto result = co_await (
    timeout.async_wait(asio::as_tuple(asio::use_awaitable))
    || (asio::async_read(formatOut, asio::dynamic_buffer(buffer), asio::as_tuple(asio::use_awaitable)) && write_to_clang_format(formatIn, chatText)));

  if (auto timeoutResult = std::get_if<0>(&result)) {
    auto [ec] = *timeoutResult;
    if (ec) {
      co_return std::nullopt;
    }
    broadcaster_->Send(Serializer<CppFormatState::Timeout>{}.Serialize(commandIdx));
    sig.emit(asio::cancellation_type::partial);
    timeout.expires_after(std::chrono::seconds(3));
    auto [exitRequest] = co_await timeout.async_wait(asio::as_tuple(asio::use_awaitable));
    if (not exitRequest) sig.emit(asio::cancellation_type::terminal);

  } else if (auto formatResult = std::get_if<1>(&result)) {
    auto &[readErr, read, writeResult] = *formatResult;
    auto &[writeErr, write]            = writeResult;
    if (readErr.value() == 109 and read > 0) {
      broadcaster_->Send(Serializer<CppFormatState::Success>{}.Serialize(commandIdx, buffer));
    } else if (readErr or writeErr) {
      fmt::println("error {}, {}", readErr.what(), writeErr.what());
      broadcaster_->Send(
        Serializer<CppFormatState::Error>{}.Serialize(commandIdx, u8"Unexpected error, report id ####"));
    }

    sig.emit(asio::cancellation_type::partial);
    timeout.expires_after(std::chrono::seconds(3));
    auto [exitRequest] = co_await timeout.async_wait(asio::as_tuple(asio::use_awaitable));
    if (not exitRequest) sig.emit(asio::cancellation_type::terminal);
  }

  co_return std::nullopt;
}
}// namespace TwitchBot