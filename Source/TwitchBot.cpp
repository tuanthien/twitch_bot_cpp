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
#include <fmt/format.h>

#include "TwitchIRCParser.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using tcp_resolver_results = typename tcp::resolver::results_type;

// Report a failure
void fail(beast::error_code ec, char const *what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
net::awaitable<void>
do_session(
    std::string host,
    std::string port,
    std::string text,
    std::string channel,
    ssl::context &ctx)
{
  // These objects perform our I/O
  auto resolver = net::use_awaitable.as_default_on(
      tcp::resolver(co_await net::this_coro::executor));

  using executor_type = net::any_io_executor;
  using executor_with_default = net::as_tuple_t<net::use_awaitable_t<executor_type>>::executor_with_default<executor_type>;
  using my_tcp_stream = beast::basic_stream<net::ip::tcp, executor_with_default, beast::unlimited_rate_policy>;
  using my_websocket = websocket::stream<beast::ssl_stream<my_tcp_stream>>;
  my_websocket ws{co_await boost::asio::this_coro::executor, ctx};

  // auto ws = websocket::stream<beast::ssl_stream<beast::tcp_stream>>(co_await net::this_coro::executor, ctx);

  // auto ws = net::use_awaitable.as_default_on(
  //     websocket::stream<
  //         beast::ssl_stream<beast::tcp_stream>>(co_await net::this_coro::executor));

  // Look up the domain name
  auto const results = co_await resolver.async_resolve(host, port);

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
  ws.set_option(
      websocket::stream_base::timeout::suggested(
          beast::role_type::client));

  // Set a decorator to change the User-Agent of the handshake
  ws.set_option(websocket::stream_base::decorator(
      [](websocket::request_type &req)
      {
        req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
      }));

  // Perform the websocket handshake
  co_await ws.async_handshake(host, "/");

  // This buffer will hold the incoming message
  using flat_u8buffer =
      beast::basic_flat_buffer<std::allocator<char8_t>>;
  flat_u8buffer buffer;

  // twitch.tv/membership twitch.tv/tags
  co_await ws.async_write(net::buffer("CAP REQ :twitch.tv/tags twitch.tv/commands\r\n"));
  co_await ws.async_write(net::buffer("NICK " + text + "\r\n"));
  co_await ws.async_write(net::buffer("JOIN #" + channel + "\r\n"));

  while (true)
  {
    // Read a message into our buffer
    co_await ws.async_read(buffer);
    auto buffer_data = reinterpret_cast<const char8_t *>(buffer.cdata().data());

    auto message = TwitchBot::Parse(std::u8string_view(buffer_data, buffer.size()));
    if (message)
    {
      auto &[command, tags, source, parameters] = message.value();
      switch (command.Kind)
      {
      case TwitchBot::IRCCommand::PRIVMSG:
      {
        auto commandParams = TwitchBot::ParseCommand<TwitchBot::IRCCommand::PRIVMSG>{}(message.value());
        if (commandParams)
        {
          auto &privmsg = parameters.value();
          auto &[displayName, msgParts] = commandParams.value();
          std::cout << std::string_view(reinterpret_cast<const char *>(displayName.data()), displayName.size())
                    << ": "
                    << std::string_view(reinterpret_cast<const char *>(privmsg.data()), privmsg.size()) << '\n';
        }
        break;
      }
      case TwitchBot::IRCCommand::PING:
      {
        auto commandParams = TwitchBot::ParseCommand<TwitchBot::IRCCommand::PING>{}(parameters);
        if (commandParams)
        {
          auto &[payload] = commandParams.value();
          auto pong = std::u8string(u8"PONG :").append(payload).append(u8"\r\n");
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

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Check command line arguments.
  if (argc != 5)
  {
    std::cerr << "Usage: websocket-client-awaitable <host> <port> <text>\n"
              << "Example:\n"
              << "    websocket-client-awaitable echo.websocket.org 80 \"Hello, world!\"\n";
    return EXIT_FAILURE;
  }
  auto const host = argv[1];
  auto const port = argv[2];
  auto const text = argv[3];
  auto const channel = argv[4];

  // The io_context is required for all I/O
  net::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::tlsv12_client};

  // This holds the root certificate used for verification
  load_root_certificates(ctx);

  // Launch the asynchronous operation
  net::co_spawn(ioc,
                do_session(host, port, text, channel, ctx),
                [](std::exception_ptr e)
                {
                  if (e)
                    try
                    {
                      std::rethrow_exception(e);
                    }
                    catch (std::exception &e)
                    {
                      std::cerr << "Error: " << e.what() << "\n";
                    }
                });

  // Run the I/O service. The call will return when
  // the socket is closed.
  ioc.run();

  return EXIT_SUCCESS;
}
