#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// Inherit from this function to implement different games
template <typename Move>
class Game {
 public:
  [[nodiscard]] virtual double Valuate() const = 0;

  virtual void MakeMove(const Move &move) = 0;

  virtual void RevertMove(const Move &move) = 0;

  [[nodiscard]] virtual std::vector<Move> GetMoves() const = 0;

  [[nodiscard]] virtual std::string ToString() const = 0;

  double MinimaxHelper(int8_t max_depth, int8_t depth, Move move) {
    MakeMove(move);
    double value;
    if (depth < max_depth) {
      // If we've not yet reached the desired depth, continue recursing,
      // alternating between selecting the minimizer and maximizer at each level
      std::vector<double> values;
      for (auto child : GetMoves()) {
        values.push_back(MinimaxHelper(max_depth, depth + 1, child));
      }
      // Take the maximum on even depths and the minimum on odd depths
      if (depth % 2 == 0) {
        value = *std::ranges::max_element(values);
      } else {
        value = *std::ranges::min_element(values);
      }
    } else {
      // When we reached the desired depth, compute material advantage
      value = Valuate();
    }
    RevertMove(move);
    return value;
  }

  // The shallowest level of the minimax search is separate because we want to
  // return the move itself instead of its value
  Move Minimax(int8_t max_depth) {
    double best_value = std::numeric_limits<double>::min();
    Move best_move;
    for (auto move : GetMoves()) {
      double value = MinimaxHelper(max_depth, 1, move);
      if (value > best_value) {
        best_value = value;
        best_move = move;
      }
    }
    return best_move;
  }
};
