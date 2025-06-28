#include <array>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "../tourney_base.hpp"

// Assign human-readable names to ANSI escape codes.
constexpr std::string kCursorHome = "\x1B[H";
constexpr std::string kEraseScreen = "\x1B[2J";

enum Piece : uint8_t { kEmpty, kX, kO };

using Square = uint8_t;

struct TicTacToeMove {
  Square square_;
};

class TicTacToe final : public Game<TicTacToeMove> {  // NOLINT
 public:
  TicTacToe() = default;

  void MakeMove(const TicTacToeMove &move) override {
    board_[move.square_] = (x_to_move_ ? kX : kO);
    x_to_move_ = !x_to_move_;
  }

  void UnmakeMove(const TicTacToeMove &move) override {
    board_[move.square_] = kEmpty;
    x_to_move_ = !x_to_move_;
  }

  [[nodiscard]] std::vector<TicTacToeMove> GetMoves() const override {
    std::vector<TicTacToeMove> moves;
    for (Square i = 0; i < 9; ++i) {
      if (board_[i] == 0) {
        moves.push_back(TicTacToeMove{.square_ = i});
      }
    }
    return moves;
  }

  [[nodiscard]] std::string ToString() const override;

  [[nodiscard]] std::optional<TicTacToeMove> Parse(
      const std::string &input) const override;

 private:
  std::array<Piece, 9> board_{};

  bool x_to_move_ = true;
};

std::string TicTacToe::ToString() const {
  static constexpr std::array<char, 3> kPieceLetters = {' ', 'X', 'O'};
  std::string output = kEraseScreen + kCursorHome;
  output +=
      std::format("{} │{} │{} \n──┼──┼──\n{} │{} │{} \n──┼──┼──\n{} │{} │{} \n",
                  kPieceLetters[board_[0]], kPieceLetters[board_[1]],
                  kPieceLetters[board_[2]], kPieceLetters[board_[3]],
                  kPieceLetters[board_[4]], kPieceLetters[board_[5]],
                  kPieceLetters[board_[6]], kPieceLetters[board_[7]],
                  kPieceLetters[board_[8]]);
  return output;
}

std::optional<TicTacToeMove> TicTacToe::Parse(const std::string &input) const {
  const auto i = static_cast<Square>(std::stoi(input));
  if (i >= 9 || board_[i] != kEmpty) {
    return std::nullopt;
  }
  return TicTacToeMove{i};
}
