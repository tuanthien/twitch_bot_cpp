#include "WebSocketSession.hpp"

#include <iostream>
#include "Broadcaster.hpp"

namespace TwitchBot {
WebSocketSession::WebSocketSession(tcp::socket &&socket, const std::shared_ptr<Broadcaster> &broadcaster)
  : ws_(std::move(socket))
  , queue_(64)
  , broadcaster_(broadcaster)
{}

WebSocketSession::~WebSocketSession()
{
  broadcaster_->Leave(this);
}

void WebSocketSession::fail(beast::error_code ec, char const *what)
{
  // Don't report these
  if (ec == net::error::operation_aborted || ec == websocket::error::closed) return;

  std::cerr << what << ": " << ec.message() << "\n";
}

void WebSocketSession::onAccept(beast::error_code ec)
{
  // Handle the error, if any
  if (ec) return fail(ec, "accept");

  // Add this session to the list of active sessions
  broadcaster_->Join(this, queue_);

  // Read a message
  ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::onRead(beast::error_code ec, std::size_t)
{
  // Handle the error, if any
  if (ec) return fail(ec, "read");

  // Send to all connections

  // Clear the buffer
  buffer_.consume(buffer_.size());

  // Read another message
  ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::Stop()
{
  net::post(ws_.get_executor(), beast::bind_front_handler(&WebSocketSession::onRequestStop, shared_from_this()));
}

void WebSocketSession::onRequestStop()
{
  beast::error_code ec;
  ws_.close(websocket::close_code::normal, ec);
  ws_.next_layer().close();
}

void WebSocketSession::NotifySend()
{
  // Post our work to the strand, this ensures
  // that the members of `this` will not be
  // accessed concurrently.

  net::post(ws_.get_executor(), beast::bind_front_handler(&WebSocketSession::onSend, shared_from_this()));
}

void WebSocketSession::onSend()
{
  if (queue_.read_available() and not writing_) {
    writing_ = true;
    ws_.async_write(
      net::buffer(queue_.front()), beast::bind_front_handler(&WebSocketSession::onWrite, shared_from_this()));
  }
}

void WebSocketSession::onWrite(beast::error_code ec, std::size_t)
{
  // Handle the error, if any
  if (ec) return fail(ec, "write");

  // Remove the string from the queue
  queue_.pop();

  // Send the next message if any
  if (queue_.read_available()) {
    ws_.async_write(
      net::buffer(queue_.front()), beast::bind_front_handler(&WebSocketSession::onWrite, shared_from_this()));
  } else {
    writing_ = false;
  }
}
}// namespace TwitchBot