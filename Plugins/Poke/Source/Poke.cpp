#include "Poke.hpp"

// #include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include "boost/asio/experimental/awaitable_operators.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/process/v2.hpp>
#include "Conversion.hpp"
#include <fmt/format.h>
#include "Broadcaster.hpp"
#include "PokeMessage.hpp"
#include "MessageSerializer.hpp"
#include <fstream>
#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

#include <filesystem>

namespace TwitchBot {

namespace proc = boost::process::v2;
namespace asio = boost::asio;

auto CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
  -> std::shared_ptr<CommandFactory>
{
  if (command == "poke") {
    return std::make_shared<PokeFactory>(ioc);
  } else {
    return nullptr;
  }
}

auto GetInfo() -> PluginInfo *
{
  static auto registry = PokeInfo();
  return static_cast<PluginInfo *>(&registry);
}

auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>
{
  co_return true;
}

auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion) -> boost::asio::awaitable<bool>
{
  co_return false;
}

auto Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
  -> boost::asio::awaitable<bool>
{
  co_return false;
}

auto PokeFactory::Create(const std::shared_ptr<Broadcaster> &broadcaster)
  -> boost::asio::awaitable<std::shared_ptr<CommandHandler>>
{
  // @TODO not hard code config
  co_return std::make_shared<Poke>(PokeConfig{.Cooldown = std::chrono::seconds(10)}, broadcaster);
}

Poke::Poke(PokeConfig &&config, const std::shared_ptr<Broadcaster> &broadcaster)
  : broadcaster_(broadcaster)
  , lastPoke_(std::chrono::utc_clock::now() - std::chrono::seconds(30))
  , config_(std::move(config))
{}

auto Poke::Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
  -> asio::awaitable<std::optional<CommandResult>>
{
  auto &[displayName, msgParts] = command;
  auto commandIdx               = std::bit_cast<intptr_t>(userData);

  broadcaster_->Send(Serialize(command, commandIdx, true));

  auto durationSinceLastPoke = std::chrono::utc_clock::now() - lastPoke_;
  const bool hasCooldowned   = durationSinceLastPoke >= config_.Cooldown;

  if (hasCooldowned) {
    lastPoke_ = std::chrono::utc_clock::now();
    broadcaster_->Send(Serializer<PokeState::InProgress>{}.Serialize(commandIdx, displayName));

    asio::steady_timer delay{co_await asio::this_coro::executor, std::chrono::seconds(1)};
    co_await delay.async_wait(asio::use_awaitable);

    broadcaster_->Send(Serializer<PokeState::Success>{}.Serialize(commandIdx));
  } else {
    const auto nextInMilis =
      std::chrono::duration_cast<std::chrono::milliseconds>((config_.Cooldown - durationSinceLastPoke)).count();
    broadcaster_->Send(Serializer<PokeState::Reject>{}.Serialize(commandIdx, nextInMilis));
  }

  co_return std::nullopt;
}
}// namespace TwitchBot