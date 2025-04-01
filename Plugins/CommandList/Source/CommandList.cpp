#include "CommandList.hpp"

// #include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include "boost/asio/experimental/awaitable_operators.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/process/v2.hpp>
#include "Conversion.hpp"
#include <fmt/format.h>
#include "Broadcaster.hpp"
#include "CommandListMessage.hpp"
#include "MessageSerializer.hpp"
#include <fstream>
#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

namespace TwitchBot {

auto CreateCommandFactory(std::string_view command, boost::asio::io_context &ioc, const ConnectParams &connectDb)
  -> std::shared_ptr<CommandFactory>
{
  if (command == "commands") {
    return std::make_shared<CommandListFactory>();
  } else {
    return nullptr;
  }
}


auto GetInfo() -> PluginInfo *
{
  static auto registry = CommandListInfo();
  return static_cast<PluginInfo *>(&registry);
}

auto DatabaseUp(boost::asio::io_context &ioc, std::u8string_view oldVersion, ConnectParams connectDb)
  -> boost::asio::awaitable<bool>
{
  co_return true;
}

auto DatabaseDown(boost::asio::io_context &ioc, std::u8string_view oldVersion) -> boost::asio::awaitable<bool>
{
  co_return true;
}

auto Configure(boost::asio::io_context &ioc, const std::vector<std::pair<std::u8string_view, FieldType>> &entries)
  -> boost::asio::awaitable<bool>
{
  co_return false;
}

auto CommandListFactory::Create(const std::shared_ptr<Broadcaster> &broadcaster)
  -> boost::asio::awaitable<std::shared_ptr<CommandHandler>>
{
  co_return std::make_shared<CommandList>(broadcaster);
}

CommandList::CommandList(const std::shared_ptr<Broadcaster> &broadcaster)
  : broadcaster_(broadcaster)
{}

namespace proc = boost::process::v2;
namespace asio = boost::asio;

auto CommandList::Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
  -> asio::awaitable<std::optional<CommandResult>>
{
  auto &[displayName, msgParts] = command;

  auto commandIdx = std::bit_cast<intptr_t>(userData);
  broadcaster_->Send(Serialize(command, commandIdx, true));
  broadcaster_->Send(Serializer<CommandListState::Querying>{}.Serialize(commandIdx));

  asio::steady_timer delay{co_await asio::this_coro::executor, std::chrono::seconds(1)};
  co_await delay.async_wait(asio::use_awaitable);


  broadcaster_->Send(Serializer<CommandListState::Success>{}.Serialize(commandIdx, u8"!commands, !cpp, !poke"));

  co_return std::nullopt;
}


}// namespace TwitchBot