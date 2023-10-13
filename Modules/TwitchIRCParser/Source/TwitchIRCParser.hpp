#pragma once

#include <string_view>
#include <optional>
#include <vector>
#include <variant>

namespace TwitchBot::IRC
{
  struct Source
  {
    std::optional<std::u8string_view> Nick;
    std::u8string_view Host;
  };

  enum struct IRCCommand
  {
    PING,
    PRIVMSG,
    UNKNOWN
  };

  struct Command
  {
    IRCCommand Kind;
    std::u8string_view Name;
    std::optional<std::u8string_view> Channel;
  };

  struct EmotePosition
  {
    std::u8string_view Id;
    int Start;
    int End;
  };

  struct EmotesInfo
  {
    std::vector<std::u8string_view> Ids;
    std::vector<EmotePosition> Positions;
  };

  struct Tags
  {
    std::optional<EmotesInfo> Emotes;
    std::optional<std::u8string_view> DisplayName;
  };

  struct TwitchIRCMessage
  {
    struct Command Command;
    std::optional<struct Tags> Tags;
    std::optional<struct Source> Source;
    std::optional<std::u8string_view> Parameters;
  };

  auto Parse(std::u8string_view ircMessage) -> std::optional<TwitchIRCMessage>;

  struct TextPart
  {
    std::u8string_view Value;
  };
  struct EmotePart
  {
    std::u8string_view Value;
  };

  using MessagePart = std::variant<TextPart, EmotePart>;

  template <IRCCommand NTCommand>
  struct CommandParameters
  {
  };

  template <IRCCommand NTCommand>
  struct ParseCommand
  {
    auto operator()() -> std::optional<CommandParameters<NTCommand>> { return {}; }
  };

  template <>
  struct CommandParameters<IRCCommand::PING>
  {
    std::u8string_view Payload;
  };

  template <>
  struct ParseCommand<IRCCommand::PING>
  {
    auto operator()(std::optional<std::u8string_view> parameters) -> std::optional<CommandParameters<IRCCommand::PING>>;
  };

  template <>
  struct CommandParameters<IRCCommand::PRIVMSG>
  {
    std::u8string_view DisplayName;
    std::vector<MessagePart> Parts;
  };

  template <>
  struct ParseCommand<IRCCommand::PRIVMSG>
  {
    auto operator()(const TwitchIRCMessage &emotesInfo) -> std::optional<CommandParameters<IRCCommand::PRIVMSG>>;
  };
}