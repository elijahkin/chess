#include <iostream>
#include <string>
#include <vector>

// Define color scheme via ANSI escape codes
const std::string kWhiteText = "\33[37m";
const std::string kBlackText = "\33[30m";
const std::string kResetText = "\33[39m";
const std::string kWhiteBackground = "\33[48;5;33m";
const std::string kBlackBackground = "\33[48;5;20m";
const std::string kResetBackground = "\33[49m";

typedef int8_t Square;

struct Move {
  Square from;
  Square to;
};

enum Color : int8_t { kWhite, kBlack };

enum PieceType : int8_t { kKing, kQueen, kRook, kBishop, kKnight, kPawn };

const std::vector<PieceType> major_rank = {kRook,  kKnight, kBishop, kKing,
                                           kQueen, kBishop, kKnight, kRook};

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
    }
    // stream << std::format("\u{:x}", 9812 + piece.type);
    stream << kResetText;
    return stream;
  }
};

class Board {
private:
  std::vector<int8_t> squares_;
  std::vector<Piece> pieces_;

public:
  // Set up the board with the pieces in their starting positions
  Board() {
    squares_.resize(64);
    for (int8_t i = 0; i < squares_.size(); ++i) {
      squares_[i] = -1;
    }

    pieces_.reserve(32);
    for (int8_t i = 0; i < 8; i++) {
      CreatePiece(kBlack, major_rank[i], i);
      CreatePiece(kBlack, kPawn, i + 8);
      CreatePiece(kWhite, kPawn, i + 48);
      CreatePiece(kWhite, major_rank[i], i + 56);
    }
  }

  friend std::ostream &operator<<(std::ostream &stream, const Board &board) {
    bool isWhiteSquare = true;
    for (int8_t rank = 0; rank < 8; ++rank) {
      stream << static_cast<int>(8 - rank);
      stream << " ";
      for (int8_t file = 0; file < 8; ++file) {
        stream << (isWhiteSquare ? kWhiteBackground : kBlackBackground);
        int8_t square = board.squares_[rank * 8 + file];
        stream << " ";
        if (square == -1) {
          stream << " ";
        } else {
          stream << board.pieces_[square];
        }
        stream << " ";
        isWhiteSquare = !isWhiteSquare;
      }
      stream << kResetBackground;
      stream << "\n";
      isWhiteSquare = !isWhiteSquare;
    }
    stream << "  ";
    for (int8_t file = 0; file < 8; ++file) {
      stream << " ";
      stream << static_cast<char>('a' + file);
      stream << " ";
    }
    return stream;
  }

  void CreatePiece(Color color, PieceType type, int8_t square) {
    squares_[square] = pieces_.size();
    pieces_.push_back({color, type, square});
  }

  // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
  // K for king, Q for queen, R for rook, B for bishop, and N for knight
  Move ParseAlgebraicNotation(std::string move) {
    // TODO do stripping if contains and x for capture or e.p. for en passant.
    // Also handle special case of castling

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

    for (auto piece : pieces_) {
      if (piece.type == piece_type) {
        std::cout << "Found piece of correct type at square "
                  << static_cast<int>(piece.square) << std::endl;
        // Is it viable?
      }
    }

    const int8_t from = 0; // TODO

    // Use file and rank from algebraic notation to compute the to index
    const int8_t to = (move[1] - 'a') * 8 + (move[2] - '0');

    return {from, to};
  }

  // TODO std::vector<Square> getRank() {}
  // TODO std::vector<Square> getFile() {}
  // TODO std::vector<Square> getDiagonal() {}
  // TODO std::vector<Square> getAntiDiagonal() {}
  // TODO bool isOccupied(Square square) {}

  std::vector<Square> LegalMovesForPiece(Piece piece) {
    std::vector<Square> moves;
    switch (piece.type) {
    case kKing:
      break;
    case kQueen:
      break;
    case kRook:
      for (int i = 0; i < 8; ++i) {
        moves.push_back(8 * (piece.square / 8) + i); // file
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
    }
    return moves;
  }
};

int main() {
  Board board;
  std::cout << board << std::endl;
  // board.ParseAlgebraicNotation("Be5");
  return 0;
}
