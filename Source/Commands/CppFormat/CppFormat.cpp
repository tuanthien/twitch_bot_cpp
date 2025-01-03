#include "CppFormat.hpp"

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
#include "CppFormatMessage.hpp"
#include "MessageSerializer.hpp"
#include <fstream>
#include <ztd/text.hpp>
#include <ztd/text/wide_execution.hpp>

namespace TwitchBot {

// namespace program_options = boost::program_options;

// program_options::options_description CppFormat::OPTIONS("Allowed options");
// program_options::positional_options_description CppFormat::POSITIONAL_OPTIONS;
// std::string CppFormat::HELP_STRING(
//   "[Command] \n !evade command set evade percentage\n"
//   " -d,display: message display mode [chat, dialog]\n"
//   " -e,enable: enable evade modifier\n"
//   " -t,threshold: threshold to activate always evade, in HP\n");
// bool CppFormat::INITIALIZED = false;

// void CppFormat::initialize()
// {
//   if (!INITIALIZED) {
//     OPTIONS.add_options()("help,h", "Display help message");
//     INITIALIZED = true;
//   }
// }

// program_options::variables_map vm;
// try {
//   text.remove_prefix(5);
//   auto options = std::string(to_string_view(text));
//   auto args    = program_options::split_unix(options);
//   auto option  = program_options::basic_command_line_parser<char>(args)
//                   .options(botCommand->Options().Description)
//                   .positional(botCommand->Options().PositionalDescription)
//                   .allow_unregistered()
//                   .run();

//   program_options::store(option, vm);
// } catch (std::exception &e) {
//   std::cout << "Command error: " << e.what() << std::endl;
// }

CppFormat::CppFormat(const std::shared_ptr<Broadcaster> &broadcaster, const BotCommandCppConfig &config)
  : broadcaster_(broadcaster)
  , config_(&config)
{}

namespace proc = boost::process::v2;
namespace asio = boost::asio;


auto CppFormat::Handle(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &command, void *userData)
  -> asio::awaitable<std::optional<CommandResult>>
{
  auto &[displayName, msgParts] = command;

  auto commandIndx = std::bit_cast<intptr_t>(userData);
  broadcaster_->Send(Serialize(command, commandIndx, true));
  broadcaster_->Send(Serializer<CppFormatState::Formatting>{}.Serialize(commandIndx));

  std::u8string_view chatText = std::get<IRC::TextPart>(msgParts[0]).Value;
  chatText.remove_prefix(5);
  std::string fileName = fmt::format("{}.cpp", commandIndx);
  auto filePath        = config_->cppTempPath / fileName;
  fileName             = filePath.string();
  auto styleFile       = fmt::format("--style=file:{}", config_->clangFormatConfigPath.string());
  /** // Beast doesn't like having epoll disabled, Process doesn't like io uring enable without epoll disabled
   *  // temporary solution, use sync file io, =.=!
     auto file = asio::stream_file(
      co_await asio::this_coro::executor,
      fileName,
      asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate);

    auto [e, size] = co_await file.async_write_some(asio::buffer(chatText), asio::as_tuple(asio::use_awaitable));
    file.cancel();
    file.close();
    co_await asio::this_coro::reset_cancellation_state();
    if (e) {
      co_return std::nullopt;
    }
  */

  // asio::steady_timer delay{co_await asio::this_coro::executor, std::chrono::seconds(1)};
  // co_await delay.async_wait(asio::use_awaitable);

  auto writeStream = std::ofstream(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
  if (writeStream.is_open()) {
    std::string_view buffer = to_string_view(chatText);
    writeStream.write(buffer.data(), buffer.size());
  }
  writeStream.close();

  asio::steady_timer timeout{co_await asio::this_coro::executor, config_->timeout};
  asio::cancellation_signal sig;
  std::u8string buffer;
  asio::readable_pipe formatOut(co_await asio::this_coro::executor);

  using namespace boost::asio::experimental::awaitable_operators;
  // clang-format off
    using result_type =
      std::variant<
        std::tuple<boost::system::error_code, int>,
        std::tuple<boost::system::error_code, size_t>,
        std::tuple<boost::system::error_code>
      >;

    auto encodeBuffer              = std::string();
    auto decodeState          = ztd::text::make_decode_state(ztd::text::wide_execution);
    auto encodeState = ztd::text::make_encode_state(ztd::text::utf8);
    auto clangFormatView = std::wstring_view(config_->clangFormatPath.c_str());
    // @TODO need to do something about this buffer
    char32_t my_intermediate_buffer[256];
    std::span<char32_t> pivot(my_intermediate_buffer);
    auto transcodeResult = ztd::text::transcode_into_raw(
    clangFormatView,
      ztd::text::wide_execution,
      ztd::ranges::unbounded_view(std::back_inserter(encodeBuffer)),
      ztd::text::utf8,
      ztd::text::replacement_handler,
      ztd::text::replacement_handler,
      decodeState,
      encodeState,
      pivot);


// const wchar_t* clangFormatPath = config_->clangFormatPath.c_str();
    result_type result = co_await (
      proc::async_execute(
        proc::process(
          co_await asio::this_coro::executor,
          "E:\\Tools\\LLVM\\19.1.0\\bin\\clang-format.exe",
          {styleFile, fileName},
          proc::process_stdio{nullptr, formatOut, {}}
        ),
        asio::bind_cancellation_slot(sig.slot(),asio::as_tuple(asio::use_awaitable))
      )
      || asio::async_read(formatOut, asio::dynamic_buffer(buffer, 1024), asio::as_tuple(asio::use_awaitable))
      || timeout.async_wait(asio::as_tuple(asio::use_awaitable)));
  // clang-format on

  if (const auto processResult = std::get_if<0>(&result)) {
    timeout.cancel();
    auto [ec, exitCode] = *processResult;
    if (ec == boost::system::errc::success && exitCode == 0) {
      broadcaster_->Send(Serializer<CppFormatState::Success>{}.Serialize(commandIndx, buffer));
    }
  } else if (const auto readResult = std::get_if<1>(&result)) {
    timeout.cancel();
    sig.emit(asio::cancellation_type::terminal);
    auto [ec, size] = *readResult;
    if (ec != boost::asio::error::misc_errors::eof) {
      broadcaster_->Send(Serializer<CppFormatState::Error>{}.Serialize(commandIndx, ec.message()));
    }
  } else if (const auto timeoutResult = std::get_if<2>(&result)) {
    auto [timeoutError] = *timeoutResult;
    if (timeoutError == boost::system::errc::success) {
      sig.emit(asio::cancellation_type::terminal);
      broadcaster_->Send(Serializer<CppFormatState::Timeout>{}.Serialize(commandIndx));
    } else {
      broadcaster_->Send(Serializer<CppFormatState::Error>{}.Serialize(commandIndx, timeoutError.message()));
    }
  }
  co_return std::nullopt;
}

auto CppFormat::HelpString() -> std::u8string_view
{
  return u8"format input as C++ code, forbid emotes";
}

}// namespace TwitchBot