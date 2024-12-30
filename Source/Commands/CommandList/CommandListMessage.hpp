#pragma once

#include <cstdint>
#include <string>

namespace TwitchBot {
enum struct CommandListState {
  Success  = 0,
  Error    = 1,
  Querying = 2,
  Timeout  = 3,
};

template<CommandListState State, typename CommandState = CommandListState>
  requires(std::same_as<CommandState, CommandListState>)
struct Serializer
{
  auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CommandListState::Success, CommandListState>
{
  auto Serialize(int64_t refId, std::u8string_view formatted) -> std::string;
};

template<>
struct Serializer<CommandListState::Error, CommandListState>
{
  auto Serialize(int64_t refId, std::string_view message) -> std::string;
};

template<>
struct Serializer<CommandListState::Querying, CommandListState>
{
  auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CommandListState::Timeout, CommandListState>
{
  auto Serialize(int64_t refId) -> std::string;
};
}// namespace TwitchBot