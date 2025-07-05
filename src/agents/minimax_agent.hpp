#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

#include "../tourney_base.hpp"

using Score = float;

// Performs the minimax algorithm with alpha-beta pruning and depth limited by
// `max_plies_`.
template <typename Move>
class MinimaxAgent final : public Agent<Move> {
 public:
  MinimaxAgent(Game<Move> &state, int max_plies,
               std::function<Score(const Move &)> heuristic_value_adjustment)
      : state_(state),
        max_plies_(max_plies),
        heuristic_value_adjustment_(heuristic_value_adjustment) {}

  Move SelectMove() override {
    std::cout << "Minimax agent is thinking...\n";
    leaf_nodes_count_ = 0;
    const auto begin = std::chrono::steady_clock::now();

    Score best_value = kNegInf;
    std::vector<Move> best_moves;
    for (const auto &move : state_.GenerateLegalMoves()) {
      const Score value = AlphaBeta(move, 1, kNegInf, kInf);
      if (value > best_value) {
        best_moves.clear();
        best_moves.push_back(move);
        best_value = value;
      } else if (value == best_value) {
        best_moves.push_back(move);
      }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
            .count();
    std::cout << "Selected move with value " << best_value << " after visiting "
              << leaf_nodes_count_ << " leaf nodes in " << elapsed_ms << "ms\n";
    return best_moves[0];
  }

 private:
  static constexpr Score kInf = std::numeric_limits<Score>::infinity();
  static constexpr Score kNegInf = -std::numeric_limits<Score>::infinity();

  // https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning#Pseudocode
  Score AlphaBeta(const Move &move, int ply, Score alpha, Score beta) {
    state_.MakeMove(move);
    heuristic_value_ += heuristic_value_adjustment_(move);

    if (ply == max_plies_) {
      const Score leaf_value = heuristic_value_;
      leaf_nodes_count_++;
      state_.UnmakeMove(move);
      heuristic_value_ -= heuristic_value_adjustment_(move);
      return leaf_value;
    }

    Score value;
    if (ply % 2 == 0) {  // Maximizing player
      value = kNegInf;
      for (const auto &child : state_.GenerateLegalMoves()) {
        value = std::max(value, AlphaBeta(child, ply + 1, alpha, beta));
        if (value >= beta) {
          break;
        }
        alpha = std::max(alpha, value);
      }
    } else {  // Minimizing player
      value = kInf;
      for (const auto &child : state_.GenerateLegalMoves()) {
        value = std::min(value, AlphaBeta(child, ply + 1, alpha, beta));
        if (value <= alpha) {
          break;
        }
        beta = std::min(beta, value);
      }
    }
    state_.UnmakeMove(move);
    heuristic_value_ -= heuristic_value_adjustment_(move);
    return value;
  }

  Game<Move> &state_;

  int max_plies_;

  Score heuristic_value_ = 0;

  std::function<Score(const Move &)> heuristic_value_adjustment_;

  size_t leaf_nodes_count_ = 0;
};
