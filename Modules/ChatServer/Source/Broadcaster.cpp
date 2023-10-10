#include "Broadcaster.hpp"
#include "WebSocketSession.hpp"

namespace TwitchBot {
auto Broadcaster::Join(WebSocketSession *session, boost::lockfree::spsc_queue<std::u8string> &queue) -> bool
{
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_[session] = &queue;
  return false;
}
void Broadcaster::Leave(WebSocketSession *session)
{
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.erase(session);
}
void Broadcaster::Send(std::u8string_view message)
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto &[session, queue] : sessions_) {
    if (not queue->write_available()) {
      continue;
    }
    queue->push(std::u8string(message));
    session->NotifySend();
  }
}
void Broadcaster::Stop()
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto &[session, _] : sessions_) {
    session->Stop();
  }
}
}// namespace TwitchBot