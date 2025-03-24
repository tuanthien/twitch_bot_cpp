#pragma once

#include <cstdint>
#include <string>
#include "CppFormatExport.hpp"

namespace TwitchBot {
enum struct CppFormatState {
  Success    = 0,
  Error      = 1,
  Formatting = 2,
  Timeout    = 3,
};

template<CppFormatState State, typename CommandState = CppFormatState>
struct Serializer
{
  CPPFORMAT_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CppFormatState::Success, CppFormatState>
{
  CPPFORMAT_API auto Serialize(int64_t refId, std::u8string_view formatted) -> std::string;
};

template<>
struct Serializer<CppFormatState::Error, CppFormatState>
{
  CPPFORMAT_API auto Serialize(int64_t refId, std::u8string_view message) -> std::string;
};

template<>
struct Serializer<CppFormatState::Formatting, CppFormatState>
{
  CPPFORMAT_API auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CppFormatState::Timeout, CppFormatState>
{
  CPPFORMAT_API auto Serialize(int64_t refId) -> std::string;
};
}// namespace TwitchBot