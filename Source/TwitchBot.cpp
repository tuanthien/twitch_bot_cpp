#include <CertificateStore.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

// #include <boost/mysql/any_address.hpp>
// #include <boost/mysql/any_connection.hpp>
// #include <boost/mysql/error_with_diagnostics.hpp>
// #include <boost/mysql/results.hpp>

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
#include "DatabaseConfig.hpp"
#include "MessageSerializer.hpp"
#include "Database.hpp"

#include "PluginRegistry.hpp"

namespace beast     = boost::beast;// from <boost/beast.hpp>
namespace http      = beast::http;// from <boost/beast/http.hpp>
namespace websocket = beast::websocket;// from <boost/beast/websocket.hpp>
namespace net       = boost::asio;// from <boost/asio.hpp>
namespace asio      = boost::asio;// from <boost/asio.hpp>
namespace ssl       = boost::asio::ssl;// from <boost/asio/ssl.hpp>
using tcp           = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>

// namespace mysql = boost::mysql;

using tcp_resolver_results = typename tcp::resolver::results_type;
using flat_u8buffer        = beast::basic_flat_buffer<std::allocator<char8_t>>;


namespace TwitchBot {

auto botCommandHandler(
  std::shared_ptr<CommandHandler> command,
  [[maybe_unused]] flat_u8buffer buffer,
  IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> message,
  void *userData) -> net::awaitable<std::pair<bool, void *>>
{
  std::optional<CommandResult> result = co_await command->Handle(message, userData);
  co_return std::pair<bool, void *>{false, userData};
}

// Sends a WebSocket message and prints the response
net::awaitable<void> do_session(
  net::io_context &ioc,
  TwitchBot::TwitchBotConfig &&config,
  ssl::context &ctx,
  std::shared_ptr<TwitchBot::Broadcaster> broadcaster,
  TwitchBot::ConnectParams connectDb)
{
  mysql::any_connection conn(ioc);
  mysql::connect_params params;
  params.server_address.emplace_host_and_port(
    std::string(reinterpret_cast<const char *>(connectDb.Host.data())), connectDb.Port);
  params.username = reinterpret_cast<const char *>(connectDb.Username.data());
  params.password = reinterpret_cast<const char *>(connectDb.Password.data());
  params.database = reinterpret_cast<const char *>(connectDb.Database.data());

  // Connect to the server
  auto mysqlConnectResult = co_await conn.async_connect(params, asio::as_tuple);
  if (std::get<0>(mysqlConnectResult) != boost::system::errc::success) {
    fmt::println(stderr, "MySQL connect: {}", std::get<0>(mysqlConnectResult).what());
    co_return;
  }
  auto registry = PluginRegistry(ioc, conn);

  auto commandList = co_await registry.LoadPlugin(
    ioc, u8"E:/Projects/twitch_bot_cpp/_out/windows-debug/Plugins/CommandList/CommandList.dll", connectDb);
  auto commandsCommandFactory = commandList->CreateCommandFactory(u8"commands", ioc, connectDb);
  auto commandsCommand        = co_await commandsCommandFactory->Create(broadcaster);
  assert(commandsCommand);// @TODO

  auto cppFormat = co_await registry.LoadPlugin(
    ioc, u8"E:/Projects/twitch_bot_cpp/_out/windows-debug/Plugins/CppFormat/CppFormat.dll", connectDb);
  auto cppCommandFactory = cppFormat->CreateCommandFactory(u8"cpp", ioc, connectDb);
  auto cppCommand        = co_await cppCommandFactory->Create(broadcaster);
  assert(cppCommand);// @TODO


  auto poke = co_await registry.LoadPlugin(
    ioc, u8"E:/Projects/twitch_bot_cpp/_out/windows-debug/Plugins/Poke/Poke.dll", connectDb);
  auto pokeCommandFactory = poke->CreateCommandFactory(u8"poke", ioc, connectDb);
  auto pokeCommand        = co_await pokeCommandFactory->Create(broadcaster);
  assert(pokeCommand);// @TODO

  std::unordered_map<std::u8string, std::array<std::shared_ptr<CommandHandler>, 8>> commandHandlers;
  commandHandlers.emplace(
    std::piecewise_construct, std::forward_as_tuple(u8"cpp"), std::forward_as_tuple(std::move(cppCommand)));

  commandHandlers.emplace(
    std::piecewise_construct, std::forward_as_tuple(u8"commands"), std::forward_as_tuple(std::move(commandsCommand)));

  commandHandlers.emplace(
    std::piecewise_construct, std::forward_as_tuple(u8"poke"), std::forward_as_tuple(std::move(pokeCommand)));

  // Print the first field in the first row
  // std::cout << result.rows().at(0).at(0) << std::endl;

  // Close the connection
  // co_await conn.async_close();

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


  // twitch.tv/membership twitch.tv/tags
  co_await ws.async_write(net::buffer("CAP REQ :twitch.tv/tags twitch.tv/commands\r\n"));
  co_await ws.async_write(net::buffer(std::string("NICK ") + nick + "\r\n"));
  co_await ws.async_write(net::buffer(std::string("JOIN #") + channel + "\r\n"));
  intptr_t commandIdx = 0;

  while (true) {
    // This buffer will hold the incoming message
    flat_u8buffer buffer;
    // Read a message into our buffer
    auto [ec, read] = co_await ws.async_read(buffer);
    if (ec) {
      fmt::println("Error async_read from twitch irc exit... {}", ec.what());
    }
    if (read == 0) continue;
    auto ircMessage = std::u8string_view(reinterpret_cast<const char8_t *>(buffer.cdata().data()), buffer.size());
    // fmt::println("IRC: {}", reinterpret_cast<const char*>(buffer_data));
    // broadcaster->Send(std::u8string_view(buffer_data, buffer.size()));
    auto message = IRC::Parse(ircMessage);
    fmt::println("IRC: {}", std::string_view(reinterpret_cast<const char *>(ircMessage.data()), ircMessage.size()));

    if (message) {
      auto &[command, tags, source, parameters] = message.value();
      switch (command.Kind) {
      case IRC::IRCCommand::PRIVMSG: {
        auto commandParams = IRC::ParseCommand<IRC::IRCCommand::PRIVMSG>{}(message.value());
        if (commandParams) {
          auto &privmsg                 = parameters.value();
          auto &[displayName, msgParts] = commandParams.value();

          bool handled = false;
          if (msgParts.size() == 1 and std::holds_alternative<IRC::TextPart>(msgParts[0])) {
            std::u8string_view chatText = std::get<IRC::TextPart>(msgParts[0]).Value;
            if (chatText.starts_with(u8'!')) {
              for (const auto &[commandText, handlers] : commandHandlers) {
                bool startWith = chatText.substr(1, commandText.size()).starts_with(commandText);
                bool hasSpace  = commandText.size() + 1 <= chatText.size() ? chatText.substr(commandText.size() + 1, 1) == u8" " : false;
                bool isSingle  = chatText.size() == commandText.size() + 1;

                if (startWith and (isSingle or (not isSingle and hasSpace))) {
                  for (auto &handler : handlers) {
                    if (not handler) continue;
                    net::co_spawn(
                      ioc,
                      botCommandHandler(
                        handler, std::move(buffer), std::move(*commandParams), reinterpret_cast<void *>(++commandIdx)),
                      [](std::exception_ptr e, std::pair<bool, void *> result) { auto [success, userData] = result; });
                    handled = true;
                  }
                }
              }
            }
          }

          if (not handled) {
            broadcaster->Send(Serialize(*commandParams, ++commandIdx));
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
  constexpr static const std::u8string_view databaseJsonUtf8Path = u8"config/database.json";
  auto databaseConfig                                            = TwitchBot::ReadDatabaseConfig(databaseJsonUtf8Path);
  if (not databaseConfig) {
    return EXIT_FAILURE;
  }


  auto broadcaster = std::make_shared<TwitchBot::Broadcaster>();

  // The io_context is required for all I/O
  net::io_context ioc;

  auto server = TwitchBot::ChatServer(
    ioc, serverConfig->Http.Host, serverConfig->Http.Port, serverConfig->Http.RootDoc, broadcaster);

  server.Start();
  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  // This holds the root certificate used for verification
  boost::system::error_code ec;

  TwitchBot::AddRootCerts(ctx);

  auto connectDb = TwitchBot::ConnectParams{
    .Host                = databaseConfig->Host,
    .Username            = databaseConfig->Username,
    .Password            = databaseConfig->Password,
    .Database            = databaseConfig->Database,
    .ConnectionCollation = 45,
    .SSL                 = true,
    .MultiQueries        = false};

  // Launch the asynchronous operation
  net::co_spawn(
    ioc, TwitchBot::do_session(ioc, std::move(*twitchConfig), ctx, broadcaster, connectDb), [](std::exception_ptr e) {
      if (e) try {
          std::rethrow_exception(e);
        } catch (std::exception &ex) {
          std::cerr << "Error: " << ex.what() << "/n";
        }
    });

  auto threadPool    = std::vector<std::jthread>(0);
  int threadPoolSize = std::thread::hardware_concurrency() - 1;
  threadPool.reserve(threadPoolSize);
  for (int i = 0; i < threadPoolSize; ++i) {
    threadPool.emplace_back([&]() { ioc.run(); });
  }

  ioc.run();

  for (auto &thread : threadPool) {
    thread.join();
  }

  return EXIT_SUCCESS;
}