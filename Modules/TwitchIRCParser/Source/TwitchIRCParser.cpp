#include "TwitchIRCParser.hpp"

#include <charconv>
#include <algorithm>

#include <iostream>
namespace TwitchBot {
struct IRCComponents
{
  std::u8string_view Tags;
  std::u8string_view Source;
  std::u8string_view Command;
  std::u8string_view Parameters;
};

static auto parseIRCCommponents(std::u8string_view ircMessage) -> IRCComponents
{
  int idxEnd = 0;
  std::u8string_view tags;
  std::u8string_view source;
  std::u8string_view command;
  std::u8string_view parameters;

  if (ircMessage.starts_with(u8'@')) {
    idxEnd = ircMessage.find(u8' ');
    tags   = ircMessage.substr(1, idxEnd);
    if (idxEnd != std::u8string_view::npos) tags.remove_suffix(1);
    ircMessage = ircMessage.substr(idxEnd + 1);
  }

  if (ircMessage.starts_with(u8':')) {
    idxEnd = ircMessage.find(u8' ');
    source = ircMessage.substr(1, idxEnd);
    if (idxEnd != std::u8string_view::npos) source.remove_suffix(1);
    ircMessage = ircMessage.substr(idxEnd + 1);
  }

  idxEnd  = ircMessage.find(u8':');
  command = ircMessage.substr(0, idxEnd);
  if (idxEnd != std::u8string_view::npos) {
    command.remove_suffix(1);
    parameters = ircMessage.substr(idxEnd + 1);
  } else {
    parameters = ircMessage.substr(ircMessage.size());
  }

  return {
    .Tags       = tags,
    .Source     = source,
    .Command    = command,
    .Parameters = parameters,
  };
}

static auto parseSource(std::u8string_view source) -> Source
{
  int splitIdx = source.find(u8'!');
  if (splitIdx == std::u8string_view::npos) {
    return {.Nick = std::nullopt, .Host = source};
  } else {
    return {.Nick = source.substr(0, splitIdx), .Host = source.substr(splitIdx + 1 /* UNSAFE */)};
  }
}

static auto parseCommand(std::u8string_view command) -> Command
{
  int splitIdx = command.find(u8' ');
  std::u8string_view name;
  std::u8string_view channel;
  IRCCommand kind = IRCCommand::UNKNOWN;

  if (splitIdx == std::u8string_view::npos) {
    name = command;
  } else {
    name    = command.substr(0, splitIdx);
    channel = command.substr(splitIdx + 1);
  }

  if (name == u8"PING") {
    kind = IRCCommand::PING;
  } else if (name == u8"PRIVMSG") {
    kind = IRCCommand::PRIVMSG;
  }

  return {.Kind = kind, .Name = name, .Channel = channel};
}

static auto parseEmotesTag(std::u8string_view emotes) -> std::optional<EmotesInfo>
{
  EmotesInfo emotesInfo;

  while (not emotes.empty()) {
    int positionIdx       = emotes.find(u8':');
    std::u8string_view id = emotes.substr(0, positionIdx);
    emotesInfo.Ids.push_back(id);
    emotes = emotes.substr(positionIdx + 1);

    int idIdx                    = emotes.find(u8'/');
    std::u8string_view positions = emotes.substr(0, idIdx);

    while (not positions.empty()) {
      int endPosIdx            = positions.find(u8'-');
      std::u8string_view start = positions.substr(0, endPosIdx);
      positions                = positions.substr(endPosIdx + 1);

      int posIdx             = positions.find(u8',');
      std::u8string_view end = positions.substr(0, posIdx);

      int startInt;
      {
        auto [ptr, ec] = std::from_chars(
          reinterpret_cast<const char *>(start.data()),
          reinterpret_cast<const char *>(start.data() + start.size()),
          startInt);
        if (ec != std::errc{}) return std::nullopt;
      }
      int endInt;
      {
        auto [ptr, ec] = std::from_chars(
          reinterpret_cast<const char *>(end.data()), reinterpret_cast<const char *>(end.data() + end.size()), endInt);
        if (ec != std::errc{}) return std::nullopt;
      }

      emotesInfo.Positions.emplace_back(id, startInt, endInt);

      if (posIdx == std::u8string_view::npos) break;

      positions = positions.substr(posIdx + 1);
    }

    if (idIdx == std::u8string_view::npos) break;

    emotes = emotes.substr(idIdx + 1);
  }
  std::sort(emotesInfo.Positions.begin(), emotesInfo.Positions.end(), [](const auto &first, const auto &second) {
    return first.End > second.End;
  });

  return emotesInfo;
}

static auto parseTags(std::u8string_view tags) -> Tags
{
  Tags resultTags;
  while (not tags.empty()) {
    int valueIdx             = tags.find(u8'=');
    std::u8string_view key   = tags.substr(0, valueIdx);
    tags                     = tags.substr(valueIdx + 1);
    int keyIdx               = tags.find(u8';');
    std::u8string_view value = tags.substr(0, keyIdx);
    tags                     = tags.substr(keyIdx + 1);

    if (value.empty()) goto no_parse;

    if (key == u8"emotes") {
      resultTags.Emotes = parseEmotesTag(value);
      if (resultTags.Emotes) {
        // @ERROR @TODO, parse emotes failed
      }
    } else if (key == u8"display-name") {
      resultTags.DisplayName = value;
    }

  no_parse:
    if (keyIdx == std::u8string_view::npos) break;
  }
  return resultTags;
}

static auto privateMessageSplitParts(std::u8string_view message, const std::optional<EmotesInfo> &emotesInfo)
  -> std::vector<MessagePart>
{
  constexpr const static auto slice = [](std::u8string_view &str, int start, int end = -1) {
    if (end != -1) return str.substr(start, end);

    const auto size = str.size();
    if (start > size) return str.substr(size);

    return str.substr(start, start - size);
  };

  if (message.size() > 2 and message.ends_with(u8"\r\n")) {
    message.remove_suffix(2);
  }

  std::vector<MessagePart> parts;
  if (emotesInfo) {
    for (const auto &position : emotesInfo->Positions) {
      std::u8string_view text = slice(message, position.End + 1);
      if (not text.empty()) parts.emplace_back(std::in_place_type<TextPart>, text);

      parts.emplace_back(std::in_place_type<EmotePart>, position.Id);

      message = slice(message, 0, position.Start);
    }
  }

  if (not message.empty()) parts.emplace_back(std::in_place_type<TextPart>, message);

  return parts;
}
auto Parse(std::u8string_view ircMessage) -> std::optional<TwitchIRCMessage>
{
  auto [tags, source, command, parameters] = parseIRCCommponents(ircMessage);

  if (command.empty()) return std::nullopt;

  TwitchIRCMessage message;
  message.Command = parseCommand(command);
  message.Tags    = tags.empty() ? std::optional<Tags>(std::nullopt) : std::optional<Tags>(parseTags(tags));
  message.Source  = source.empty() ? std::optional<Source>(std::nullopt) : std::optional<Source>(parseSource(source));
  message.Parameters = parameters.empty() ? std::optional<std::u8string_view>(std::nullopt)
                                          : std::optional<std::u8string_view>(parameters);
  return message;
}

auto ParseCommand<IRCCommand::PING>::operator()(std::optional<std::u8string_view> parameters)
  -> std::optional<CommandParameters<IRCCommand::PING>>
{
  if (not parameters) return std::nullopt;
  return CommandParameters<IRCCommand::PING>{.Payload = parameters.value()};
}

auto ParseCommand<IRCCommand::PRIVMSG>::operator()(const TwitchIRCMessage &message)
  -> std::optional<CommandParameters<IRCCommand::PRIVMSG>>
{
  auto &[command, tags, source, parameters] = message;
  std::u8string_view displayName;
  if (not parameters) return std::nullopt;

  if (tags) {
    auto &[emotes, tagDisplayName] = tags.value();
    if (tagDisplayName) displayName = tagDisplayName.value();
  } else {
    if (source and source.value().Nick) displayName = source.value().Nick.value();
  }

  return CommandParameters<IRCCommand::PRIVMSG>{
    .DisplayName = displayName,
    .Parts       = privateMessageSplitParts(parameters.value(), tags ? tags->Emotes : std::nullopt)};
}
}// namespace TwitchBot