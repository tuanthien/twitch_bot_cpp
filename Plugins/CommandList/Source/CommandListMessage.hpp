#pragma once

#include <cstdint>
#include <string>
#include "CommandListExport.hpp"

namespace TwitchBot {
enum struct CommandListState {
  Success    = 0,
  Error      = 1,
  Querying = 2,
  Timeout    = 3,
};

template<CommandListState State, typename CommandState = CommandListState>
struct Serializer
{
  COMMANDLIST_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CommandListState::Success, CommandListState>
{
  COMMANDLIST_API auto Serialize(int64_t refId, std::u8string_view formatted) -> std::string;
};

template<>
struct Serializer<CommandListState::Error, CommandListState>
{
  COMMANDLIST_API auto Serialize(int64_t refId, std::u8string_view message) -> std::string;
};

template<>
struct Serializer<CommandListState::Querying, CommandListState>
{
  COMMANDLIST_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CommandListState::Timeout, CommandListState>
{
  COMMANDLIST_API auto Serialize(int64_t refId) -> std::string;
};
}// namespace TwitchBot