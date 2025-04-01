#pragma once

#include <cstdint>
#include <span>
#include <limits>
#include <cassert>

namespace TwitchBot {
struct [[nodiscard]] Xoshiro256StarStar
{
public:
  using result_type = uint64_t;

  constexpr explicit Xoshiro256StarStar(const std::span<const uint64_t, std::dynamic_extent> state) noexcept
  {
    assert((state.size() >= 4) && "Xoshiro256StarStar not enough bits to initialize state");
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }
  constexpr explicit Xoshiro256StarStar(const std::span<const uint64_t, 4> state) noexcept
  {
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }
  constexpr explicit Xoshiro256StarStar(
    uint64_t partialState1, uint64_t partialState2, uint64_t partialState3, uint64_t partialState4) noexcept
  {
    state_[0] = partialState1;
    state_[1] = partialState2;
    state_[2] = partialState3;
    state_[3] = partialState4;
  }

  [[nodiscard]] constexpr result_type operator()() noexcept
  {

    constexpr auto RotL = [](const uint64_t x, const int s) noexcept {
      return (x << s) | (x >> (64 - s));
    };

    const uint64_t result = RotL(state_[1] * 5, 7) * 9;
    const uint64_t t      = state_[1] << 17;
    state_[2] ^= state_[0];
    state_[3] ^= state_[1];
    state_[1] ^= state_[2];
    state_[0] ^= state_[3];
    state_[2] ^= t;
    state_[3] = RotL(state_[3], 45);
    return result;
  }

  // This is the jump function for the generator. It is equivalent
  // to 2^128 calls to next(); it can be used to generate 2^128
  // non-overlapping subsequences for parallel computations.
  constexpr void Jump() noexcept
  {
    constexpr uint64_t JUMP[] = {
      0x18'0e'c6'd3'3c'fd'0a'ba, 0xd5'a6'12'66'f0'c9'39'2c, 0xa9'58'26'18'e0'3f'c9'aa, 0x39'ab'dc'45'29'b1'66'1c};

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;

    for (uint64_t jump : JUMP) {
      for (int b = 0; b < 64; ++b) {
        if (jump & uint64_t{1} << b) {
          s0 ^= state_[0];
          s1 ^= state_[1];
          s2 ^= state_[2];
          s3 ^= state_[3];
        }
        std::ignore = operator()();
      }
    }

    state_[0] = s0;
    state_[1] = s1;
    state_[2] = s2;
    state_[3] = s3;
  }

  // This is the long-jump function for the generator. It is equivalent to
  // 2^192 calls to next(); it can be used to generate 2^64 starting points,
  // from each of which jump() will generate 2^64 non-overlapping
  // subsequences for parallel distributed computations.
  constexpr void LongJump() noexcept
  {
    constexpr uint64_t LONG_JUMP[] = {
      0x76'e1'5d'3e'fe'fd'cb'bf, 0xc5'00'4e'44'1c'52'2f'b3, 0x77'71'00'69'85'4e'e2'41, 0x39'10'9b'b0'2a'cb'e6'35};

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;

    for (uint64_t jump : LONG_JUMP) {
      for (int b = 0; b < 64; ++b) {
        if (jump & uint64_t{1} << b) {
          s0 ^= state_[0];
          s1 ^= state_[1];
          s2 ^= state_[2];
          s3 ^= state_[3];
        }
        std::ignore = operator()();
      }
    }

    state_[0] = s0;
    state_[1] = s1;
    state_[2] = s2;
    state_[3] = s3;
  }

  [[nodiscard]] constexpr auto GetState() const noexcept -> std::span<const uint64_t, 4>
  {
    return std::span<const uint64_t, 4>(state_);
  }

  constexpr auto SetState(const std::span<const uint64_t, std::dynamic_extent> state) noexcept
  {
    assert((state.size() >= 4) && "Xoshiro256StarStar not enough bits to initialize state");
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }

  constexpr auto SetState(const std::span<const uint64_t, 4> state) noexcept
  {
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }

  constexpr auto
    SetState(uint64_t partialState1, uint64_t partialState2, uint64_t partialState3, uint64_t partialState4) noexcept
  {
    state_[0] = partialState1;
    state_[1] = partialState2;
    state_[2] = partialState3;
    state_[3] = partialState4;
  }

  [[nodiscard]] static constexpr result_type Min() noexcept { return min(); }

  [[nodiscard]] static constexpr result_type Max() noexcept { return max(); }

  [[nodiscard]] static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::lowest(); }

  [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

  [[nodiscard]] friend constexpr auto operator==(const Xoshiro256StarStar &lhs, const Xoshiro256StarStar &rhs) noexcept
    -> bool
  {
    return (
      lhs.state_[0] == rhs.state_[0] and lhs.state_[1] == rhs.state_[1] and lhs.state_[2] == rhs.state_[2]
      and lhs.state_[3] == rhs.state_[3]);
  }

  [[nodiscard]] friend constexpr auto operator!=(const Xoshiro256StarStar &lhs, const Xoshiro256StarStar &rhs) noexcept
    -> bool
  {
    return (
      lhs.state_[0] != rhs.state_[0] or lhs.state_[1] != rhs.state_[1] or lhs.state_[2] != rhs.state_[2]
      or lhs.state_[3] != rhs.state_[3]);
  }

private:
  uint64_t state_[4];
};

}// namespace TwitchBot
