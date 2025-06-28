#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <limits>
#include <vector>

#include "../tourney_base.hpp"

// Performs the minimax algorithm with alpha-beta pruning and depth limited by
// `max_plies_`.
template <typename Move>
class MinimaxAgent final : public Agent<Move> {
 public:
  using Score = float;

  explicit MinimaxAgent(int max_plies) : max_plies_(max_plies) {}

  Move SelectMove(Game<Move> &state) override {
    std::cout << "Minimax agent is thinking...\n";
    leaf_nodes_count_ = 0;
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
    std::cout << "Selected move with value " << best_value << " after visiting "
              << leaf_nodes_count_ << " leaf nodes in " << elapsed_ms << "ms\n";
    return best_moves[0];
  }

 private:
  static constexpr Score kInf = std::numeric_limits<Score>::infinity();
  static constexpr Score kNegInf = -std::numeric_limits<Score>::infinity();

  static Score HeuristicValueAdjustment(const Move &move) {
    constexpr std::array<Score, 13> kBlackAdvantageOnCapture = {
        0, 200, 9, 5, 3, 3, 1, -200, -9, -5, -3, -3, -1};

    return kBlackAdvantageOnCapture[move.captured];
  }

  // https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning#Pseudocode
  Score AlphaBeta(Game<Move> &state, const Move &move, int ply, Score alpha,
                  Score beta) {
    state.MakeMove(move);
    heuristic_value_ += HeuristicValueAdjustment(move);

    if (ply == max_plies_) {
      const Score leaf_value = heuristic_value_;
      leaf_nodes_count_++;
      state.UnmakeMove(move);
      heuristic_value_ -= HeuristicValueAdjustment(move);
      return leaf_value;
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
    heuristic_value_ -= HeuristicValueAdjustment(move);
    return value;
  }

  int max_plies_;

  Score heuristic_value_ = 0;

  size_t leaf_nodes_count_ = 0;
};
