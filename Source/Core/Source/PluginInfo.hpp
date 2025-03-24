#pragma once
#include <string_view>
#include <span>
#include <utility>
#include <variant>
#include <cstdint>
#include <string>

namespace TwitchBot {

using FieldType = std::variant<bool, int32_t, uint32_t, std::string>;

enum struct FieldDescription {
  Bool = 0,
  Int = 1,
  UInt = 2,
  String = 3,
};

struct PluginInfo
{
  virtual auto Id() const -> std::u8string_view                                                             = 0;
  virtual auto Name() const -> std::u8string_view                                                           = 0;
  virtual auto Version() const -> std::u8string_view                                                        = 0;
  virtual auto BotVersion() const -> std::u8string_view                                                     = 0;
  virtual auto Help() const -> std::u8string_view                                                           = 0;
  virtual auto Description() const -> std::u8string_view                                                    = 0;
  virtual auto ConfigDescriptor() const -> std::span<const std::pair<std::u8string_view, FieldDescription>> = 0;
};

}// namespace TwitchBot