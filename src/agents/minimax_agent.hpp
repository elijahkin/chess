#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <vector>

#include "../base.hpp"

// Performs the minimax algorithm with alpha-beta pruning and depth limited by
// `max_plies_`.
template <typename Move>
class MinimaxAgent final : public Agent<Move> {
 public:
  explicit MinimaxAgent(int max_plies) : max_plies_(max_plies) {}

  Move SelectMove(Game<Move> &state) override {
    std::cout << "Minimax agent is thinking...\n";
    evaluations_ = 0;
    const auto begin = std::chrono::steady_clock::now();

    Score best_value = kNegInf;
    std::vector<Move> best_moves;
    for (const auto &move : state.GetMoves()) {
      const Score value = AlphaBeta(state, move, 1, kNegInf, kInf);
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
    std::cout << "Selected move with value " << best_value << " after "
              << evaluations_ << " evaluations in " << elapsed_ms << "ms\n";
    return best_moves[0];
  }

 private:
  static constexpr Score kInf = std::numeric_limits<Score>::infinity();
  static constexpr Score kNegInf = -std::numeric_limits<Score>::infinity();

  // https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning#Pseudocode
  Score AlphaBeta(Game<Move> &state, const Move &move, int ply, Score alpha,
                  Score beta) {
    state.MakeMove(move);

    if (ply == max_plies_) {
      Score value = state.HeuristicValue();
      evaluations_++;
      state.UnmakeMove(move);
      return value;
    }

    Score value;
    if (ply % 2 == 0) {  // Maximizing player
      value = kNegInf;
      for (const auto &child : state.GetMoves()) {
        value = std::max(value, AlphaBeta(state, child, ply + 1, alpha, beta));
        if (value >= beta) {
          break;
        }
        alpha = std::max(alpha, value);
      }
    } else {  // Minimizing player
      value = kInf;
      for (const auto &child : state.GetMoves()) {
        value = std::min(value, AlphaBeta(state, child, ply + 1, alpha, beta));
        if (value <= alpha) {
          break;
        }
        beta = std::min(beta, value);
      }
    }
    state.UnmakeMove(move);
    return value;
  }

  int max_plies_;

  int evaluations_ = 0;
};
