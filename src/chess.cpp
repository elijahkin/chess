#include <iostream>
#include <string>
#include <vector>
// #include <format>

// Define color scheme via ANSI escape codes
const std::string kWhiteText = "\33[37m";
const std::string kBlackText = "\33[30m";
const std::string kResetText = "\33[39m";
const std::string kWhiteBackground = "\33[48;5;33m";
const std::string kBlackBackground = "\33[48;5;20m";
const std::string kResetBackground = "\33[49m";

enum Color : int8_t { kWhite, kBlack };

enum PieceType : int8_t {
  kNone,
  kKing,
  kQueen,
  kRook,
  kBishop,
  kKnight,
  kPawn
};

typedef int8_t Square;

struct Piece {
  Color color;
  PieceType type;
  Square square;

  // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
  friend std::ostream &operator<<(std::ostream &stream, const Piece &piece) {
    stream << ((piece.color == kWhite) ? kWhiteText : kBlackText);
    switch (piece.type) {
    case kKing:
      stream << "\u2654";
      break;
    case kQueen:
      stream << "\u2655";
      break;
    case kRook:
      stream << "\u2656";
      break;
    case kBishop:
      stream << "\u2657";
      break;
    case kKnight:
      stream << "\u2658";
      break;
    case kPawn:
      stream << "\u2659";
      break;
    case kNone:
      stream << " ";
      break;
    }
    // TODO use format with c++2a
    // stream << std::format("\u{:x}", 9812 + piece.type);
    stream << kResetText;
    return stream;
  }
};

class Chess {
private:
  // TODO switch to std::array
  std::vector<Piece> board_;
  Color colorToMove_;
  std::vector<std::string> history_;

public:
  // Set up the board with the pieces in their starting positions
  Chess() {
    board_.resize(64);
    const std::vector<PieceType> major_rank = {kRook,  kKnight, kBishop, kKing,
                                               kQueen, kBishop, kKnight, kRook};
    for (int8_t i = 0; i < 8; ++i) {
      CreatePiece(kBlack, major_rank[i], i);
      CreatePiece(kBlack, kPawn, i + 8);
      CreatePiece(kWhite, kPawn, i + 48);
      CreatePiece(kWhite, major_rank[i], i + 56);
    }

    colorToMove_ = kWhite;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Chess &game) {
    stream << '\n';
    // Print the board with rank labels on the left
    for (int8_t rank = 0; rank < 8; ++rank) {
      stream << static_cast<int>(8 - rank) << ' ';
      for (int8_t file = 0; file < 8; ++file) {
        stream << ((rank + file) % 2 ? kBlackBackground : kWhiteBackground);
        stream << ' ';
        stream << game.board_[rank * 8 + file];
        stream << ' ';
      }
      stream << kResetBackground << '\n';
    }
    // Print file labels
    stream << "  ";
    for (char file = 'a'; file <= 'h'; ++file) {
      stream << ' ' << file << ' ';
    }
    // Finally, print out the move history
    stream << '\n' << std::endl;
    for (size_t i = 0; i < game.history_.size(); ++i) {
      stream << (i + 1) << ". " << game.history_[i] << ' ';
    }
    stream << '\n';
    return stream;
  }

  void CreatePiece(Color color, PieceType type, int8_t square) {
    board_[square] = {color, type, square};
  }

  struct Move {
    Square from;
    Square to;
  };

  // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
  // K for king, Q for queen, R for rook, B for bishop, and N for knight
  Move ParseAlgebraicNotation(const std::string move) {
    // TODO do stripping if contains and x for capture or e.p. for en passant.
    // Also handle special case of castling
    assert('a' <= move[1] && move[1] <= 'h');
    assert('1' <= move[2] && move[2] <= '8');

    PieceType piece_type;
    switch (move[0]) {
    case 'K':
      piece_type = kKing;
      break;
    case 'Q':
      piece_type = kQueen;
      break;
    case 'R':
      piece_type = kRook;
      break;
    case 'B':
      piece_type = kBishop;
      break;
    case 'N':
      piece_type = kKnight;
      break;
    default:
      piece_type = kPawn;
      break;
    }

    std::vector<Square> candidates;
    for (auto piece : board_) {
      if (piece.type == piece_type && piece.color == colorToMove_) {
        // TODO Is it viable?
        candidates.push_back(piece.square);
      }
    }

    assert(candidates.size() == 1);
    const int8_t from = candidates[0];
    const int8_t to = (move[1] - 'a') + 8 * ('8' - move[2]);

    history_.push_back(move);
    return {from, to};
  }

  // TODO std::vector<Square> getRank() {}
  // TODO std::vector<Square> getFile() {}
  // TODO std::vector<Square> getDiagonal() {}
  // TODO std::vector<Square> getAntiDiagonal() {}

  std::vector<Square> LegalMovesForPiece(Piece piece) {
    std::vector<Square> moves;
    switch (piece.type) {
    case kKing:
      break;
    case kQueen:
      break;
    case kRook:
      for (int i = 0; i < 8; ++i) {
        Square square = 8 * (piece.square / 8) + i;
        moves.push_back(square); // file
        // if occupied, break
      }
      for (int i = 0; i < 8; ++i) {
        moves.push_back(8 * i + (piece.square % 8)); // rank
        // if occupied, break
      }
      break;
    case kBishop:
      break;
    case kKnight:
      break;
    case kPawn:
      moves.push_back(piece.square + 1);
      moves.push_back(piece.square + 2); // TODO two jump
      moves.push_back(piece.square - 7); // TODO attack left
      moves.push_back(piece.square + 9); // TODO attack right
      break;
    case kNone:
      break;
    }
    return moves;
  }

  // TODO should first verify that the move is legal
  // Write the move to memory and switch to the other player
  inline void MakeMove(Square from, Square to) {
    board_[to] = board_[from];
    board_[from].type = kNone;
    colorToMove_ = ((colorToMove_ == kWhite) ? kBlack : kWhite);
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
