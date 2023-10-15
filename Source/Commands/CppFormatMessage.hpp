#pragma once

#include <cstdint>
#include <string>

namespace TwitchBot {
enum struct CppFormatState {
  Success,
  Error,
  WritingFile,
  LaunchingFormatter,
};

template<CppFormatState State>
struct Serializer
{
  auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CppFormatState::Success>
{
  auto Serialize(int64_t refId, std::u8string_view formatted) -> std::string;
};

template<>
struct Serializer<CppFormatState::Error>
{
  auto Serialize(int64_t refId, std::u8string_view formatted) -> std::string;
};

template<>
struct Serializer<CppFormatState::WritingFile>
{
  auto Serialize(int64_t refId) -> std::string;
};

template<>
struct Serializer<CppFormatState::LaunchingFormatter>
{
  auto Serialize(int64_t refId) -> std::string;
};
}// namespace TwitchBot