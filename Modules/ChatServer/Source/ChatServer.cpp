#include "ChatServer.hpp"
#include "Listener.hpp"
#include "Broadcaster.hpp"
#include "HttpSession.hpp"

namespace TwitchBot {

ChatServer::ChatServer(
  net::io_context &ioc,
  std::u8string_view host,
  unsigned short port,
  std::u8string_view documentRoot,
  const std::shared_ptr<Broadcaster> &broadcaster)
  : ioc_(&ioc)
  , host_(host)
  , port_(port)
  , documentRoot_(documentRoot)
  , broadcaster_(broadcaster)
  , listener_(nullptr){};

void ChatServer::Start()
{
  if (listening_) return;

  auto ipAddress = net::ip::make_address(reinterpret_cast<const char *>(host_.c_str()));
  listener_      = std::make_shared<Listener>(*ioc_, tcp::endpoint{ipAddress, port_}, broadcaster_, *this);
  listener_->Run();

  listening_ = true;
};

void ChatServer::Stop()
{
  if (requestShutdown_) return;

  listener_->Stop();
  broadcaster_->Stop();
  listening_ = false;
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto &session : httpSessions_) {
    session->Stop();
  }
  requestShutdown_ = true;
}

void ChatServer::Join(HttpSession *httpSession)
{
  std::lock_guard<std::mutex> lock(mutex_);
  httpSessions_.insert(httpSession);
}
void ChatServer::Leave(HttpSession *httpSession)
{
  std::lock_guard<std::mutex> lock(mutex_);
  httpSessions_.erase(httpSession);
}
}// namespace TwitchBot