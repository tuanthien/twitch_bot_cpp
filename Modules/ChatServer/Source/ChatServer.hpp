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
using tcp           = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>

#include <string>
#include <memory>
#include <atomic>
#include <unordered_set>
#include <mutex>

namespace TwitchBot {

struct Broadcaster;
struct Listener;
struct HttpSession;

struct ChatServer
{
private:
  net::io_context *ioc_;
  std::u8string host_;
  unsigned short port_;
  std::u8string documentRoot_;
  std::shared_ptr<Broadcaster> broadcaster_;
  bool listening_       = false;
  bool requestShutdown_ = false;
  std::shared_ptr<Listener> listener_;
  std::unordered_set<HttpSession *> httpSessions_;
  std::mutex mutex_;

public:
  explicit ChatServer(
    net::io_context &ioc,
    std::u8string_view host,
    unsigned short port,
    std::u8string_view documentRoot,
    const std::shared_ptr<Broadcaster> &broadcaster);

  void Start();
  void Stop();
  void Join(HttpSession *httpSession);
  void Leave(HttpSession *httpSession);
};

};// namespace TwitchBot