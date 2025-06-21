#pragma once

#include <string>
#include <vector>

// Inherit from this class to implement different games
template <typename Move>
class Game {
 public:
  [[nodiscard]] virtual double Valuate() const = 0;

  virtual void MakeMove(const Move &move) = 0;

  virtual void RevertMove(const Move &move) = 0;

  [[nodiscard]] virtual std::vector<Move> GetMoves() const = 0;

  [[nodiscard]] virtual std::string ToString() const = 0;
};
