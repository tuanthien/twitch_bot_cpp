#pragma once

#include <string>
#include <string_view>
#include "Command.hpp"


namespace TwitchBot {

class CppFormat : public Command
{
public:
  CppFormat();
  auto Handle(std::u8string_view command) -> CommandResult override;
  auto HelpString() -> std::u8string_view override;
private:

};

}