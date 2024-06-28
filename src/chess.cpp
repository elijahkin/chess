// #include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
// #include <format>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

// Define color scheme via ANSI escape codes
const char kWhiteText[] = "\33[37m";
const char kBlackText[] = "\33[30m";
const char kResetText[] = "\33[39m";
const char kWhiteBackground[] = "\33[48;5;33m";
const char kBlackBackground[] = "\33[48;5;20m";
const char kResetBackground[] = "\33[49m";

enum Type : int8_t { kNone, kKing, kQueen, kRook, kBishop, kKnight, kPawn };

typedef int8_t Square;

struct Piece {
  bool is_white;
  Type type;
};

class Chess {
private:
  // In memory, we store the board rank-major such that the elements of board_
  // correspond to the squares like so: 1a ... 1h 2a ... 2h ... 7h 8a ... 8h
  // TODO switch to std::array<Piece, 64>
  std::vector<Piece> board_;
  // This boolean keeps track of whose turn it is
  bool white_to_move_;
  // We record each move in algebraic notation in this vector
  std::vector<std::string> history_;

public:
  // Set up the board with the pieces in their starting positions. The ranks are
  // numbered starting from white's side of the board, such that white's pieces
  // start in ranks 1 and 2.
  Chess() {
    board_.resize(64);
    const std::vector<Type> major_rank = {kRook,  kKnight, kBishop, kKing,
                                          kQueen, kBishop, kKnight, kRook};
    for (Square i = 0; i < 8; ++i) {
      CreatePiece(true, major_rank[i], i);
      CreatePiece(true, kPawn, i + 8);
      CreatePiece(false, kPawn, i + 48);
      CreatePiece(false, major_rank[i], i + 56);
    }
    white_to_move_ = true;
  }

  void CreatePiece(bool is_white, Type type, Square square) {
    board_[square] = {is_white, type};
  }

  std::string PieceString(Piece piece) const {
    // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
    const std::vector<std::string> unicode_pieces = {
        " ", "\u2654", "\u2655", "\u2656", "\u2657", "\u2658", "\u2659"};
    std::string output;
    output += (piece.is_white ? kWhiteText : kBlackText);
    output += unicode_pieces[piece.type];
    return output;
  }

  // Print the board with rank labels on the left
  std::string BoardString() const {
    std::string output;
    for (int8_t rank = 0; rank < 8; ++rank) {
      output += '1' + rank;
      output += ' ';
      for (int8_t file = 0; file < 8; ++file) {
        output += ((rank + file) % 2 ? kBlackBackground : kWhiteBackground);
        output += ' ';
        output += PieceString(board_[rank * 8 + file]);
        output += ' ';
      }
      output += kResetBackground;
      output += kResetText;
      output += '\n';
    }
    // Print file labels
    output += "  ";
    for (char file = 'h'; file >= 'a'; --file) {
      output += ' ';
      output += file;
      output += ' ';
    }
    return output;
  }

  std::string HistoryString() const {
    std::string output;
    for (size_t i = 0; i < history_.size(); ++i) {
      output += '1' + i;
      output += ". ";
      output += history_[i];
      output += ' ';
    }
    return output;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Chess &game) {
    stream << '\n';
    stream << game.BoardString();
    stream << "\n\n";
    stream << game.HistoryString();
    return stream;
  }

  struct Move {
    Square from;
    Square to;
  };

  // Perform the move in memory and switch with the other player. We return the
  // previous Piece on Square to in case we have to revet the move later (such
  // as during a minimax search)
  // TODO Should verify the move is legal before this
  inline Piece MakeMove(Square from, Square to) {
    Piece captured = board_[to];

    board_[to] = board_[from];
    board_[from].type = kNone;

    white_to_move_ = !white_to_move_;
    return captured;
  }

  inline void RevertMove(Square from, Square to, Piece captured) {
    board_[from] = board_[to];
    board_[to] = captured;
    white_to_move_ = !white_to_move_;
  }

  int MinimaxHelper(int8_t max_depth, int8_t depth, Move move) {
    // Make the move, storing any captured piece
    Piece captured = MakeMove(move.from, move.to);
    int value;
    if (depth < max_depth) {
      // If we've not yet reached the desired depth, continue recursing,
      // alternating between selecting the minimizer and maximizer at each level
      std::vector<int> values;
      for (auto child : LegalMoves()) {
        values.push_back(MinimaxHelper(max_depth, depth + 1, child));
      }
      // Take the maximum on even depths and the minimum on odd depths
      if (depth % 2 == 0) {
        value = *std::max_element(values.begin(), values.end());
      } else {
        value = *std::min_element(values.begin(), values.end());
      }
    } else {
      // When we reached the desired depth, compute material advantage
      value = MaterialAdvantage();
    }
    // Revert the move and return its value
    RevertMove(move.from, move.to, captured);
    return value;
  }

  // The shallowest level of the minimax search is separate because we want to
  // return the move itself instead of its value
  Move Minimax(int8_t max_depth) {
    int best_value = std::numeric_limits<int>::min();
    Move best_move;
    for (auto move : LegalMoves()) {
      int value = MinimaxHelper(max_depth, 1, move);
      if (value > best_value) {
        best_value = value;
        best_move = move;
      }
    }
    return best_move;
  }

  // Compute the sum of white's material minus the sum of black's material
  int MaterialAdvantage() {
    // TODO
    return 0;
  }

  // TODO
  std::vector<Move> LegalMoves() {
    // TODO
    return {};
  }

  // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
  Move ParseAlgebraicNotation(const std::string move) {
    // TODO do stripping if contains and x for capture or e.p. for en passant.
    // Also handle special case of castling
    assert('a' <= move[1] && move[1] <= 'h');
    assert('1' <= move[2] && move[2] <= '8');

    Type type;
    switch (move[0]) {
    case 'K':
      type = kKing;
      break;
    case 'Q':
      type = kQueen;
      break;
    case 'R':
      type = kRook;
      break;
    case 'B':
      type = kBishop;
      break;
    case 'N':
      type = kKnight;
      break;
    default:
      type = kPawn;
      break;
    }

    std::vector<Square> candidates;
    for (Square i = 0; i < board_.size(); ++i) {
      if (board_[i].type == type && board_[i].is_white == white_to_move_) {
        // TODO Is it viable?
        candidates.push_back(i);
      }
    }

    assert(candidates.size() == 1);
    const Square from = candidates[0];
    const Square to = ('h' - move[1]) + 8 * (move[2] - '1');

    history_.push_back(move);
    return {from, to};
  }
};

int main() {
  Chess game;
  std::string input;
  while (true) {
    // Clear the screen and print out the board with history
    system("clear");
    std::cout << game << std::endl;

    // Get use input for the move
    // TODO overload >> instead of having a function
    std::cout << "Please enter a move: ";
    std::cin >> input;
    Chess::Move move = game.ParseAlgebraicNotation(input);

    game.MakeMove(move.from, move.to);
  }
  return 0;
}
