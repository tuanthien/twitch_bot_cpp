#include "Listener.hpp"
#include "HttpSession.hpp"
#include <iostream>

#include <boost/asio/strand.hpp>
namespace TwitchBot {
Listener::Listener(
  net::io_context &ioc, tcp::endpoint endpoint, const std::shared_ptr<Broadcaster> &broadcaster, ChatServer &chatServer)
  : ioc_(ioc)
  , acceptor_(ioc)
  , broadcaster_(broadcaster)
  , chatServer_(&chatServer)
{
  beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return;
  }
}

void Listener::Run()
{
  // The new connection gets its own strand
  acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

void Listener::Stop()
{
  beast::error_code ec;
  acceptor_.cancel(ec);
  acceptor_.close(ec);
}

// Report a failure
void Listener::fail(beast::error_code ec, char const *what)
{
  // Don't report on canceled operations
  if (ec == net::error::operation_aborted) return;
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void Listener::onAccept(beast::error_code ec, tcp::socket socket)
{
  if (ec) {
    return fail(ec, "accept");
  } else {
    // Launch a new session for this connection
    std::make_shared<HttpSession>(std::move(socket), broadcaster_, *chatServer_)->Run();
  }
  // The new connection gets its own strand
  acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}
}// namespace TwitchBot