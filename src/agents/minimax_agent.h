#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

#include "../base.h"

static const double kInf = std::numeric_limits<double>::infinity();
static const double kNegInf = -std::numeric_limits<double>::infinity();

// Implements the minimax algorithm for abstract two-player adversarial games.
template <typename Move>
class MinimaxAgent : public Agent<Move> {
 public:
  explicit MinimaxAgent(int max_plies) : max_plies_(max_plies) {}

  // The shallowest level of the minimax search is separate because we want to
  // return the move itself instead of its value
  // TODO Consider using iterative deepening instead
  Move SelectMove(Game<Move> &state) override {
    std::cout << "Minimax agent is thinking...\n";
    double best_value = kNegInf;
    std::vector<Move> best_moves;

    for (const auto &move : state.GetMoves()) {
      const double value = AlphaBeta(state, move, 1, kNegInf, kInf);
      if (value > best_value) {
        best_moves.clear();
        best_moves.push_back(move);
        best_value = value;
      } else if (value == best_value) {
        best_moves.push_back(move);
      }
    }
    // TODO If there are multiple optimal moves, we may want to randomly select
    // among them
    return best_moves[0];
  }

 private:
  const int max_plies_;

  // A helper function for performing the minimax algorithm with alpha-beta
  // pruning, taking heavy inspiration from
  // https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning#Pseudocode
  double AlphaBeta(Game<Move> &state, const Move &move, int ply, double alpha,
                   double beta) {
    state.MakeMove(move);

    if (ply == max_plies_) {
      // TODO Consider using transposition tables
      double value = state.Valuate();
      state.RevertMove(move);
      return value;
    }

    double value;
    if (ply % 2 == 0) {  // Maximizing player
      value = kNegInf;
      for (const auto &move : state.GetMoves()) {
        value = std::max(value, AlphaBeta(state, move, ply + 1, alpha, beta));
        if (value >= beta) {
          break;
        }
        alpha = std::max(alpha, value);
      }
    } else {  // Minimizing player
      value = kInf;
      for (const auto &move : state.GetMoves()) {
        value = std::min(value, AlphaBeta(state, move, ply + 1, alpha, beta));
        if (value <= alpha) {
          break;
        }
        beta = std::min(beta, value);
      }
    }
    state.RevertMove(move);
    return value;
  }
};
