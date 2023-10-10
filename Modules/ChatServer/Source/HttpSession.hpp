#pragma once


#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast     = boost::beast;// from <boost/beast.hpp>
namespace http      = beast::http;// from <boost/beast/http.hpp>
namespace websocket = beast::websocket;// from <boost/beast/websocket.hpp>
namespace net       = boost::asio;// from <boost/asio.hpp>
namespace ssl       = boost::asio::ssl;// from <boost/asio/ssl.hpp>
using tcp           = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>


#include <optional>
#include <cstdlib>
#include <memory>

namespace TwitchBot {
struct Broadcaster;
struct ChatServer;

struct HttpSession : public std::enable_shared_from_this<HttpSession>
{
private:
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<Broadcaster> broadcaster_;
  ChatServer *chatServer_;
  // The parser is stored in an optional container so we can
  // construct it from scratch it at the beginning of each new message.
  std::optional<http::request_parser<http::string_body>> parser_;

  struct send_lambda;

  void fail(beast::error_code ec, char const *what);
  void doRead();
  void onRead(beast::error_code ec, std::size_t);
  void onWrite(beast::error_code ec, std::size_t, bool close);

public:
  HttpSession(tcp::socket &&socket, const std::shared_ptr<Broadcaster> &broadcaster, ChatServer &chatServer);
  ~HttpSession();
  void Run();
  void Stop();
};
}// namespace TwitchBot