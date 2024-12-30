#include "CommandList.hpp"

// #include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include "boost/asio/experimental/awaitable_operators.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/readable_pipe.hpp>
#include "Conversion.hpp"
#include <fmt/format.h>
#include "Broadcaster.hpp"
#include "MessageSerializer.hpp"
#include "CommandListMessage.hpp"
#include <fstream>
#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

namespace TwitchBot {

CommandList::CommandList(const std::shared_ptr<Broadcaster> &broadcaster, const BotCommandCommandsConfig &config)
  : broadcaster_(broadcaster)
  , config_(&config)
{}

namespace asio = boost::asio;


auto CommandList::Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
  -> asio::awaitable<std::optional<CommandResult>>
{
  auto &[displayName, msgParts] = command;

  auto commandIndx = std::bit_cast<intptr_t>(userData);
  broadcaster_->Send(Serialize(command, commandIndx, true));
  broadcaster_->Send(Serializer<CommandListState::Querying>{}.Serialize(commandIndx));

  std::u8string_view chatText = std::get<IRC::TextPart>(msgParts[0]).Value;
  chatText.remove_prefix(5);

  asio::steady_timer timeout{co_await asio::this_coro::executor, config_->timeout};
  asio::cancellation_signal sig;

  using namespace boost::asio::experimental::awaitable_operators;

  auto queryCommands = [](auto &&cancelation) -> asio::awaitable<std::tuple<boost::system::error_code, int>> {
    co_return std::make_tuple(boost::system::error_code{}, 0);
  };

  // clang-format off
  using result_type =
    std::variant<
      std::tuple<boost::system::error_code, int>,
      std::tuple<boost::system::error_code>
    >;
    
  result_type result = co_await (
    queryCommands(asio::bind_cancellation_slot(sig.slot(),asio::as_tuple(asio::use_awaitable)))
    || timeout.async_wait(asio::as_tuple(asio::use_awaitable)));
  // clang-format on

  if (const auto processResult = std::get_if<0>(&result)) {
    timeout.cancel();
    auto [ec, exitCode] = *processResult;
    if (ec == boost::system::errc::success && exitCode == 0) {
      broadcaster_->Send(Serializer<CommandListState::Success>{}.Serialize(commandIndx, u8"!commands, !cpp"));
    }
  } else if (const auto timeoutResult = std::get_if<1>(&result)) {
    auto [timeoutError] = *timeoutResult;
    if (timeoutError == boost::system::errc::success) {
      sig.emit(asio::cancellation_type::terminal);
      broadcaster_->Send(Serializer<CommandListState::Timeout>{}.Serialize(commandIndx));
    } else {
      broadcaster_->Send(Serializer<CommandListState::Error>{}.Serialize(commandIndx, timeoutError.message()));
    }
  }
  co_return std::nullopt;
}

auto CommandList::HelpString() -> std::u8string_view
{
  return u8"list commands available to request user";
}

}// namespace TwitchBot