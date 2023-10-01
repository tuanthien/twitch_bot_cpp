#include <gtest/gtest.h>

#include "TwitchIRCParser.hpp"

namespace TwitchBot::Test
{
  TEST(ParseTest, PrivateMessageTest)
  {
    auto message = Parse(u8"@badge-info=subscriber/6;badges=vip/1,subscriber/6;client-nonce=e6ec70fda5db9d9589de3b15a64dc3e8;color=;display-name=whaatsuuup;emote-only=1;emotes=emotesv2_dcd06b30a5c24f6eb871e8f5edbd44f7:0-8/160402:12-19;first-msg=0;flags=;id=ca7caa8e-9887-4bed-857a-e4bda9a0042e;mod=0;returning-chatter=0;room-id=176050880;subscriber=1;tmi-sent-ts=1696120512138;turbo=0;user-id=240125503;user-type=;vip=1 :whaatsuuup!whaatsuuup@whaatsuuup.tmi.twitch.tv PRIVMSG #quantumapprentice :DinoDance   SabaPin");
    ASSERT_TRUE(message);

    auto &[command, tags, source, parameters] = message.value();
    ASSERT_TRUE(tags);
    ASSERT_TRUE(source);
    ASSERT_TRUE(parameters);

    ASSERT_EQ(command.Kind, IRCCommand::PRIVMSG);

    auto &[emotes, _] = tags.value();
    ASSERT_TRUE(emotes);

    auto commandParameters = ParseCommand<IRCCommand::PRIVMSG>{}(message.value());
    ASSERT_TRUE(commandParameters);

    auto &[displayName, parts] = commandParameters.value();
    ASSERT_EQ(displayName, u8"whaatsuuup");
    ASSERT_EQ(std::get<EmotePart>(parts[0]).Value, u8"160402");
    ASSERT_EQ(std::get<TextPart>(parts[1]).Value, u8"   ");
    ASSERT_EQ(std::get<EmotePart>(parts[2]).Value, u8"emotesv2_dcd06b30a5c24f6eb871e8f5edbd44f7");
  }
  TEST(ParseTest, PingTest)
  {
    auto message = Parse(u8"PING :tmi.twitch.tv");
    ASSERT_TRUE(message);

    auto &[command, tags, source, parameters] = message.value();
    ASSERT_FALSE(tags);
    ASSERT_FALSE(source);
    ASSERT_TRUE(parameters);
    ASSERT_EQ(command.Kind, IRCCommand::PING);

    auto commandParameters = ParseCommand<IRCCommand::PING>{}(parameters);

    ASSERT_TRUE(commandParameters);
    auto &[payload] = commandParameters.value();
    ASSERT_EQ(payload, u8"tmi.twitch.tv");
  }
}
