#pragma once

#include <optional>
#include <string>
#include <vector>

// Defines the necessary functions to implement a game.
template <typename Move>
class Game {  // NOLINT
 public:
  virtual ~Game() = default;

  virtual void MakeMove(const Move &move) = 0;

  virtual void UnmakeMove(const Move &move) = 0;

  [[nodiscard]] virtual std::vector<Move> GenerateLegalMoves() const = 0;

  [[nodiscard]] virtual std::string ToString() const = 0;

  [[nodiscard]] virtual std::optional<Move> Parse(
      const std::string &input) const = 0;
};

// Defines the necessary functions to implement an agent.
template <typename Move>
class Agent {  // NOLINT
 public:
  explicit Agent(Game<Move> &state) : state_(state) {}

  virtual ~Agent() = default;

  [[nodiscard]] virtual Move SelectMove() = 0;

 protected:
  Game<Move> &state_;  // NOLINT
};
