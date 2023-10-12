#pragma once

#include "TwitchIRCParser.hpp"
#include <string>

namespace TwitchBot {
  
// @TODO move this some where
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };


auto Serialize(const CommandParameters<IRCCommand::PRIVMSG> &parameters) -> std::string;

}