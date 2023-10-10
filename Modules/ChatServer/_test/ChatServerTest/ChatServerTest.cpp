#include <gtest/gtest.h>
#include "ChatServer.hpp"
#include "Broadcaster.hpp"

namespace TwitchBot::Test {
TEST(ChatServerTest, InitializeTest)
{
  int threadCount  = 4;
  auto broadcaster = std::make_shared<Broadcaster>();

  net::io_context ioc{std::max(1, threadCount)};
  auto server = ChatServer(ioc, u8"0.0.0.0", 8040u, u8"http", broadcaster);
  server.Start();

  boost::asio::steady_timer timer(ioc, boost::asio::chrono::seconds(10));

  timer.async_wait([&](const boost::system::error_code &) { server.Stop(); });

  std::vector<std::thread> threads;
  threads.reserve(threadCount - 1);

  for (auto i = threadCount - 1; i > 0; --i) threads.emplace_back([&ioc] { ioc.run(); });
  ioc.run();

  for (auto &thread : threads) {
    thread.join();
  }
}
}// namespace TwitchBot::Test
