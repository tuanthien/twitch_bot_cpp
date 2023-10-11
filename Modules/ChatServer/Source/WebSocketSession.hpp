#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace beast     = boost::beast;// from <boost/beast.hpp>
namespace http      = beast::http;// from <boost/beast/http.hpp>
namespace websocket = beast::websocket;// from <boost/beast/websocket.hpp>
namespace net       = boost::asio;// from <boost/asio.hpp>
using tcp           = boost::asio::ip::tcp;// from <boost/asio/ip/tcp.hpp>

namespace TwitchBot {
struct Broadcaster;

struct WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
private:
  beast::flat_buffer buffer_;
  websocket::stream<beast::tcp_stream> ws_;
  boost::lockfree::spsc_queue<std::u8string> queue_;
  std::shared_ptr<Broadcaster> broadcaster_;
  void fail(beast::error_code ec, char const *what);
  void onAccept(beast::error_code ec);
  void onRead(beast::error_code ec, std::size_t bytes_transferred);
  void onWrite(beast::error_code ec, std::size_t bytes_transferred);
  void onSend();
  void onRequestStop();

public:
  WebSocketSession(tcp::socket &&socket, const std::shared_ptr<Broadcaster> &broadcaster);

  ~WebSocketSession();

  template<class Body, class Allocator>
  void Run(http::request<Body, http::basic_fields<Allocator>> req);

  void NotifySend();
  void Stop();
};

template<class Body, class Allocator>
void WebSocketSession::Run(http::request<Body, http::basic_fields<Allocator>> req)
{
  // Set suggested timeout settings for the websocket
  ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

  // Set a decorator to change the Server of the handshake
  ws_.set_option(websocket::stream_base::decorator([](websocket::response_type &res) {
    res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-chat-multi");
  }));

  // Accept the websocket handshake
  ws_.async_accept(req, beast::bind_front_handler(&WebSocketSession::onAccept, shared_from_this()));
}


}// namespace TwitchBot