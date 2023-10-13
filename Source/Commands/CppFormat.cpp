#include "CppFormat.hpp"

// #include <boost/program_options.hpp>

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

CppFormat::CppFormat()
{
  // initialize();
}

auto CppFormat::Handle(std::u8string_view command) -> CommandResult
{
  return CommandResult{SystemMessageType::Generic, command};
}

auto CppFormat::HelpString() -> std::u8string_view {
  return u8"[Command] !cpp command format input as C++ code, forbid emotes.";
}

}// namespace TwitchBot