#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <mutex>
#include <unordered_map>
#include <string>

namespace TwitchBot {

struct WebSocketSession;

struct Broadcaster
{
private:
  std::unordered_map<WebSocketSession *, boost::lockfree::spsc_queue<std::u8string> *> sessions_;
  std::mutex mutex_;

public:
  auto Join(WebSocketSession *session, boost::lockfree::spsc_queue<std::u8string> &queue) -> bool;
  void Leave(WebSocketSession *session);
  void Send(std::u8string_view message);
  void Send(const std::string& message);
  void Stop();
};

}// namespace TwitchBot