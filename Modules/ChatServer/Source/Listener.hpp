#pragma once

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>

namespace beast     = boost::beast;// from <boost/beast.hpp>
namespace http      = beast::http;// from <boost/beast/http.hpp>
namespace websocket = beast::websocket;// from <boost/beast/websocket.hpp>
namespace net       = boost::asio;// from <boost/asio.hpp>
using tcp           = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>
namespace TwitchBot {
struct Broadcaster;
struct ChatServer;

struct Listener : public std::enable_shared_from_this<Listener>
{
private:
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  std::shared_ptr<Broadcaster> broadcaster_;
  ChatServer *chatServer_;

  void fail(beast::error_code ec, char const *what);
  void onAccept(beast::error_code ec, tcp::socket socket);

public:
  Listener(
    net::io_context &ioc,
    tcp::endpoint endpoint,
    const std::shared_ptr<Broadcaster> &broadcaster,
    ChatServer &chatServer);

  // Start accepting incoming connections
  void Run();
  void Stop();
};
}// namespace TwitchBot