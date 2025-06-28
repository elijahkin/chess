#pragma once

#include <optional>
#include <string>
#include <vector>

using Score = float;

// Defines the necessary functions to implement a game.
template <typename Move>
class Game {
 public:
  virtual ~Game() = default;

  [[nodiscard]] virtual Score HeuristicValue() const = 0;

  virtual void MakeMove(const Move &move) = 0;

  virtual void UnmakeMove(const Move &move) = 0;

  [[nodiscard]] virtual std::vector<Move> GetMoves() const = 0;

  [[nodiscard]] virtual std::string ToString() const = 0;

  [[nodiscard]] virtual std::optional<Move> Parse(
      const std::string &move) const = 0;
};

// Defines the necessary functions to implement an agent.
template <typename Move>
class Agent {
 public:
  virtual ~Agent() = default;

  [[nodiscard]] virtual Move SelectMove(Game<Move> &state) = 0;
};
