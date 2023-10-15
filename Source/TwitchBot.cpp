#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "Certificates.hpp"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string_view>
#include <thread>
#include <limits>
#include <utility>

#include "Conversion.hpp"
#include "TwitchIRCParser.hpp"
#include "ChatServer.hpp"
#include "Broadcaster.hpp"
#include "TwitchBotConfig.hpp"
#include "ServerConfig.hpp"
#include "MessageSerializer.hpp"
#include "Commands/CppFormat.hpp"
// #include <boost/program_options.hpp>

#include <boost/asio.hpp>
#include "boost/asio/experimental/awaitable_operators.hpp"
#include <boost/process/v2.hpp>
#include "Conversion.hpp"
#include <fmt/format.h>
#include "Broadcaster.hpp"
#include "Commands/CppFormatMessage.hpp"
#include "MessageSerializer.hpp"

namespace TwitchBot {

namespace beast            = boost::beast;// from <boost/beast.hpp>
namespace http             = beast::http;// from <boost/beast/http.hpp>
namespace websocket        = beast::websocket;// from <boost/beast/websocket.hpp>
namespace net              = boost::asio;// from <boost/asio.hpp>
namespace asio             = boost::asio;// from <boost/asio.hpp>
namespace proc             = boost::process::v2;
namespace ssl              = boost::asio::ssl;// from <boost/asio/ssl.hpp>
using tcp                  = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>
using tcp_resolver_results = typename tcp::resolver::results_type;

// Report a failure
void fail(beast::error_code ec, char const *what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

auto botCommandHandler(
  std::unique_ptr<Command> command, IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> message, void *userData)
  -> net::awaitable<std::pair<bool, void *>>
{
  std::optional<CommandResult> result = co_await command->Handle(message, userData);
  co_return std::pair<bool, void *>{false, userData};
}

// Sends a WebSocket message and prints the response
net::awaitable<void> do_session(
  net::io_context &ioc,
  TwitchBot::TwitchBotConfig &&config,
  ssl::context &ctx,
  std::shared_ptr<TwitchBot::Broadcaster> broadcaster)
{
  {
    auto file = asio::stream_file(
      co_await asio::this_coro::executor,
      "test.cpp",
      asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate);

    auto [e, size] =
      co_await file.async_write_some(asio::buffer("int main() {return 0;}"), asio::as_tuple(asio::use_awaitable));
    file.cancel();
    file.close();
    co_await asio::this_coro::reset_cancellation_state();
    if (e) {
      co_return;
    }

    asio::steady_timer timeout{co_await asio::this_coro::executor, std::chrono::milliseconds(1000)};
    asio::cancellation_signal sig;
    std::u8string buffer;
    asio::readable_pipe formatOut(co_await asio::this_coro::executor);

    using namespace boost::asio::experimental::awaitable_operators;
    // clang-format off
    using result_type =
      std::variant<
        std::tuple<boost::system::error_code, int>,
        std::tuple<boost::system::error_code, unsigned long>,
        std::tuple<boost::system::error_code>
      >;

    result_type result = co_await (
      proc::async_execute(
        proc::process(
          co_await asio::this_coro::executor,
          "/usr/bin/clang-format",
          {"--style=GNU", "test.cpp"},
          proc::process_stdio{nullptr, formatOut, {}}
        ),
        asio::bind_cancellation_slot(sig.slot(),asio::as_tuple(asio::use_awaitable))
      )
      || asio::async_read(formatOut, asio::dynamic_buffer(buffer, 1024), asio::as_tuple(asio::use_awaitable))
      || timeout.async_wait(asio::as_tuple(asio::use_awaitable)));
    // clang-format on

    if (const auto processResult = std::get_if<0>(&result)) {
      timeout.cancel();
      auto [ec, exitCode] = *processResult;
      if (ec == boost::system::errc::success && exitCode == 0) {
        fmt::print("Success {}\n", to_string_view(buffer));
      }
    } else if (const auto readResult = std::get_if<1>(&result)) {
      timeout.cancel();
      sig.emit(asio::cancellation_type::terminal);
    } else if (const auto timeoutResult = std::get_if<2>(&result)) {
      auto [timeoutError] = *timeoutResult;
      if (timeoutError == boost::system::errc::success) sig.emit(asio::cancellation_type::terminal);
    }
  }
  // These objects perform our I/O
  auto resolver = net::use_awaitable.as_default_on(tcp::resolver(co_await net::this_coro::executor));

  using executor_type = net::any_io_executor;
  using executor_with_default =
    net::as_tuple_t<net::use_awaitable_t<executor_type>>::executor_with_default<executor_type>;
  using my_tcp_stream = beast::basic_stream<net::ip::tcp, executor_with_default, beast::unlimited_rate_policy>;
  using my_websocket  = websocket::stream<beast::ssl_stream<my_tcp_stream>>;
  my_websocket ws{co_await boost::asio::this_coro::executor, ctx};

  // auto ws = websocket::stream<beast::ssl_stream<beast::tcp_stream>>(co_await net::this_coro::executor, ctx);

  // auto ws = net::use_awaitable.as_default_on(
  //     websocket::stream<
  //         beast::ssl_stream<beast::tcp_stream>>(co_await net::this_coro::executor));

  auto &[host, port, channel, nick] = config.Connection;
  auto portStr                      = fmt::format("{}", port);
  // Look up the domain name
  auto const results = co_await resolver.async_resolve(host, portStr);

  // Set a timeout on the operation
  beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

  // Make the connection on the IP address we get from a lookup
  auto ep = co_await beast::get_lowest_layer(ws).async_connect(results);
  co_await ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client);

  // Update the host_ string. This will provide the value of the
  // Host HTTP header during the WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  host += ':' + std::to_string(std::get<1>(ep).port());

  // Turn off the timeout on the tcp_stream, because
  // the websocket stream has its own timeout system.
  beast::get_lowest_layer(ws).expires_never();

  // Set suggested timeout settings for the websocket
  ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

  // Set a decorator to change the User-Agent of the handshake
  ws.set_option(websocket::stream_base::decorator([](websocket::request_type &req) {
    req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
  }));

  // Perform the websocket handshake
  co_await ws.async_handshake(host, "/");

  // This buffer will hold the incoming message
  using flat_u8buffer = beast::basic_flat_buffer<std::allocator<char8_t>>;
  flat_u8buffer buffer;

  // twitch.tv/membership twitch.tv/tags
  co_await ws.async_write(net::buffer("CAP REQ :twitch.tv/tags twitch.tv/commands\r\n"));
  co_await ws.async_write(net::buffer(std::string("NICK ") + nick + "\r\n"));
  co_await ws.async_write(net::buffer(std::string("JOIN #") + channel + "\r\n"));
  intptr_t commandIndx = 0;

  while (true) {
    // Read a message into our buffer
    co_await ws.async_read(buffer);
    auto buffer_data = reinterpret_cast<const char8_t *>(buffer.cdata().data());
    // broadcaster->Send(std::u8string_view(buffer_data, buffer.size()));
    auto message = IRC::Parse(std::u8string_view(buffer_data, buffer.size()));
    if (message) {
      auto &[command, tags, source, parameters] = message.value();
      switch (command.Kind) {
      case IRC::IRCCommand::PRIVMSG: {
        auto commandParams = IRC::ParseCommand<IRC::IRCCommand::PRIVMSG>{}(message.value());
        if (commandParams) {
          auto &privmsg                 = parameters.value();
          auto &[displayName, msgParts] = commandParams.value();

          std::cout << std::string_view(reinterpret_cast<const char *>(displayName.data()), displayName.size()) << ": "
                    << std::string_view(reinterpret_cast<const char *>(privmsg.data()), privmsg.size()) << '\n';
          if (msgParts.size() == 1 and std::holds_alternative<IRC::TextPart>(msgParts[0])) {
            std::u8string_view chatText = std::get<IRC::TextPart>(msgParts[0]).Value;
            if (chatText.starts_with(u8"!cpp ")) {
              std::unique_ptr<Command> botCommand = std::make_unique<CppFormat>(broadcaster);
              // net::co_spawn(
              //   ioc,
              try {
                co_await botCommandHandler(
                  std::move(botCommand), std::move(*commandParams), reinterpret_cast<void *>(++commandIndx));//,
              } catch (const std::exception &ex) {
                fmt::print("Bot error {}", ex.what());
              }
              // [](std::exception_ptr e, std::pair<bool, void *> result) {
              //   auto [success, userData] = result;
              // });
            }
          }
        }
      } break;
      case IRC::IRCCommand::PING: {
        auto commandParams = IRC::ParseCommand<IRC::IRCCommand::PING>{}(parameters);
        if (commandParams) {
          auto &[payload] = commandParams.value();
          auto pong       = std::u8string(u8"PONG :").append(payload).append(u8"\r\n");
          co_await ws.async_write(net::buffer(pong));
        }
        break;
      }
      }
    }

    // std::cout << beast::make_printable(buffer.data()) << std::endl;
    buffer.clear();
  }

  // Close the WebSocket connection
  co_await ws.async_close(websocket::close_code::normal);

  // If we get here then the connection is closed gracefully
}

}// namespace TwitchBot


//------------------------------------------------------------------------------
int main()
{

  constexpr static const std::u8string_view twitchJsonUtf8Path = u8"config/twitch.json";
  auto twitchConfig                                            = TwitchBot::ReadTwitchBotConfig(twitchJsonUtf8Path);
  if (not twitchConfig) {
    return EXIT_FAILURE;
  }
  constexpr static const std::u8string_view serverJsonUtf8Path = u8"config/server.json";
  auto serverConfig                                            = TwitchBot::ReadServerConfig(serverJsonUtf8Path);
  if (not serverConfig) {
    return EXIT_FAILURE;
  }

  auto broadcaster = std::make_shared<TwitchBot::Broadcaster>();

  // The io_context is required for all I/O
  net::io_context chat_ioc;

  auto server = TwitchBot::ChatServer(
    chat_ioc, serverConfig->Http.Host, serverConfig->Http.Port, serverConfig->Http.RootDoc, broadcaster);

  server.Start();
  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  // This holds the root certificate used for verification
  boost::system::error_code ec;
  LoadRootCertificates(ctx, ec);
  net::io_context bot_ioc;

  // Launch the asynchronous operation
  net::co_spawn(
    bot_ioc, TwitchBot::do_session(bot_ioc, std::move(*twitchConfig), ctx, broadcaster), [](std::exception_ptr e) {
      if (e) try {
          std::rethrow_exception(e);
        } catch (std::exception &ex) {
          std::cerr << "Error: " << ex.what() << "\n";
        }
    });

  std::jthread chat_thread([&]() { chat_ioc.run(); });

  // Run the I/O service. The call will return when
  // the socket is closed.
  bot_ioc.run();

  chat_thread.join();

  return EXIT_SUCCESS;
}