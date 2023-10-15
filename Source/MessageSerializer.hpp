#pragma once

#include "TwitchIRCParser.hpp"
#include <string>

namespace TwitchBot {
  
// @TODO move this some where
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };


auto Serialize(const IRC::CommandParameters<IRC::IRCCommand::PRIVMSG> &parameters, int64_t serializeId) -> std::string;

}