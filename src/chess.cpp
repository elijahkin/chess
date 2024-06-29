#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
// #include <format>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

enum Piece : int8_t {
  kNone,
  kWhiteKing,
  kWhiteQueen,
  kWhiteRook,
  kWhiteBishop,
  kWhiteKnight,
  kWhitePawn,
  kBlackKing,
  kBlackQueen,
  kBlackRook,
  kBlackBishop,
  kBlackKnight,
  kBlackPawn
};

bool IsWhite(const Piece piece) {
  return (piece >= kWhiteKing && piece <= kWhitePawn);
}

class Chess {
private:
  // In memory, we store the board rank-major such that the squares laid out in
  // board_ like so: 1a ... 1h 2a ... 2h ... 7h 8a ... 8h. This makes it easier
  // to initialize the board in its starting position
  std::array<Piece, 64> board_;
  // This boolean keeps track of whose turn it is
  bool white_to_move_;
  // We record the algebraic notation of each move in this vector
  std::vector<std::string> history_;

public:
  Chess() {
    // Define the types and order of pieces in white's major rank. The same
    // order is copied for black's major rank
    const std::vector<Piece> white_major = {
        kWhiteRook,  kWhiteKnight, kWhiteBishop, kWhiteKing,
        kWhiteQueen, kWhiteBishop, kWhiteKnight, kWhiteRook};

    // Set up the board with the pieces in their starting positions. The ranks
    // are numbered starting from white's side of the board, such that white's
    // pieces start in ranks 1 and 2
    for (int8_t rank = 1; rank <= 8; ++rank) {
      for (char file = 'a'; file <= 'h'; ++file) {
        Piece piece;
        if (rank == 1 || rank == 8) {
          piece = white_major[file - 'a'];
        } else if (rank == 2 || rank == 7) {
          piece = kWhitePawn;
        } else {
          piece = kNone;
        }
        if (rank == 7 || rank == 8) {
          piece = static_cast<Piece>(piece + 6);
        }
        CreatePiece(piece, file, rank);
      }
    }
    white_to_move_ = true;
  }

  void CreatePiece(Piece piece, char file, int8_t rank) {
    board_[8 * (rank - 1) + (file - 'a')] = piece;
  }

  // TODO add a perspective_ field to determine how we print the board
  friend std::ostream &operator<<(std::ostream &stream, const Chess &game) {
    // Define color scheme via ANSI escape codes
    const char kPieceText[] = "\33[30m";
    const char kHistoryText[] = "\33[38;5;240m";
    const char kResetText[] = "\33[39m";
    const char kWhiteBackground[] = "\33[47m";
    const char kBlackBackground[] = "\33[45m";
    const char kResetBackground[] = "\33[49m";

    // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
    const std::vector<std::string> unicode_pieces = {
        " ",      "\u2654", "\u2655", "\u2656", "\u2657", "\u2658", "\u2659",
        "\u265a", "\u265b", "\u265c", "\u265d", "\u265e", "\u265f"};

    // For each rank, print out the rank label on the left, then the squares of
    // that rank, then every eighth move in the move history
    std::string output;
    for (int8_t rank = 0; rank < 8; ++rank) {
      output += '1' + rank;
      output += ' ';
      output += kPieceText;
      for (int8_t file = 0; file < 8; ++file) {
        output += ((rank + file) % 2 ? kBlackBackground : kWhiteBackground);
        output += unicode_pieces[game.board_[rank * 8 + file]];
        output += ' ';
      }
      output += kResetBackground;
      // Print out every eighth move in the move history offset by rank
      output += kHistoryText;
      for (size_t move = rank; move < game.history_.size(); move += 8) {
        output += "  ";
        // Accommodate for move numbers up to 999
        std::string move_num_str = std::to_string(move + 1);
        move_num_str.insert(0, 3 - move_num_str.size(), ' ');
        output += move_num_str;
        output += ". ";
        // TODO Pad to support algebraic notation of different lengths
        output += game.history_[move];
      }
      output += kResetText;
      output += '\n';
    }
    // It remains to print out the file labels on the bottom
    output += "  ";
    for (char file = 'h'; file >= 'a'; --file) {
      output += file;
      output += ' ';
    }
    stream << output;
    return stream;
  }

  typedef int8_t Square;

  struct Move {
    Square from;
    Square to;
  };

  // Perform the move in memory and switch with the other player. We return the
  // previous Piece on Square to in case we have to revet the move later (such
  // as during a minimax search)
  inline Piece MakeMove(Square from, Square to) {
    Piece captured = board_[to];

    board_[to] = board_[from];
    board_[from] = kNone;

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

  std::vector<Move> LegalMoves() {
    // TODO
    return {};
  }

  Move ParseAlgebraicNotation(const std::string move) {
    // TODO Strip out x for capture or e.p. for en passant.
    // TODO Also handle special case of castling
    assert('a' <= move[1] && move[1] <= 'h');
    assert('1' <= move[2] && move[2] <= '8');

    // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
    // We read off the type of the piece being moved and its destination
    int8_t offset;
    switch (move[0]) {
    case 'K':
      offset = 1;
      break;
    case 'Q':
      offset = 2;
      break;
    case 'R':
      offset = 3;
      break;
    case 'B':
      offset = 4;
      break;
    case 'N':
      offset = 5;
      break;
    default:
      offset = 6;
      break;
    }
    const Piece type = static_cast<Piece>(6 * !white_to_move_ + offset);
    const Square to = ('h' - move[1]) + 8 * (move[2] - '1');

    // Now we look for pieces of that type that can moved to the destination
    std::vector<Square> candidates;
    for (size_t i = 0; i < board_.size(); ++i) {
      if (board_[i] == type && IsWhite(board_[i]) == white_to_move_) {
        // TODO Is it viable?
        candidates.push_back(i);
      }
    }
    // The piece to move should now be unambiguous
    assert(candidates.size() == 1);
    history_.push_back(move);
    return {candidates[0], to};
  }
};

int main() {
  Chess game;
  std::string input;
  while (true) {
    // Clear the screen and print out the board with history
    system("clear");
    std::cout << game << std::endl;

    // Allow the user to input a move
    std::cout << "\nPlease enter a move: ";
    // TODO Overload >> instead of having a function?
    std::cin >> input;
    Chess::Move move = game.ParseAlgebraicNotation(input);
    game.MakeMove(move.from, move.to);
  }
  return 0;
}
