#pragma once

#include <cstdint>
#include <string>
#include "PokeExport.hpp"

namespace TwitchBot {
enum struct PokeState {
  Success    = 0,
  Error      = 1,
  InProgress = 2,
  Timeout    = 3,
  Reject       = 4,
};

template<PokeState State, typename CommandState = PokeState>
struct Serializer
{
  POKE_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<PokeState::Success, PokeState>
{
  POKE_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<PokeState::Error, PokeState>
{
  POKE_API auto Serialize(int64_t refId, std::u8string_view message) -> std::string;
};

template<>
struct Serializer<PokeState::InProgress, PokeState>
{
  POKE_API auto Serialize(int64_t refId, std::u8string_view from) -> std::string;
};

template<>
struct Serializer<PokeState::Timeout, PokeState>
{
  POKE_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<PokeState::Reject, PokeState>
{
  POKE_API auto Serialize(int64_t refId, uint64_t nextInMillis) -> std::string;
};

}// namespace TwitchBot