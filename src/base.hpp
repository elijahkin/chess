#pragma once

#include <optional>
#include <string>
#include <vector>

// Inherit from this class to implement different games
template <typename Move>
class Game {
 public:
  [[nodiscard]] virtual double HeuristicValue() const = 0;

  virtual void MakeMove(const Move &move) = 0;

  virtual void UnmakeMove(const Move &move) = 0;

  [[nodiscard]] virtual std::vector<Move> GetMoves() const = 0;

  [[nodiscard]] virtual std::string ToString() const = 0;

  [[nodiscard]] virtual std::optional<Move> Parse(std::string move) const = 0;
};

// Inherit from this class to implement differet agents
template <typename Move>
class Agent {
 public:
  [[nodiscard]] virtual Move SelectMove(Game<Move> &state) = 0;
};
