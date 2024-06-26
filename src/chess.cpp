#include <iostream>
#include <string>
#include <vector>
// #include <format>

typedef int8_t Square;

struct Move {
  Square from;
  Square to;
};

enum Color : int8_t { White, Black };

enum PieceType : int8_t { King, Queen, Rook, Bishop, Knight, Pawn };

struct Piece {
  Color color;
  PieceType type;
  Square square;

  // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
  friend std::ostream &operator<<(std::ostream &stream, const Piece &piece) {
    stream << ((piece.color == White) ? "\33[31m" : "\33[32m");
    switch (piece.type) {
    case King:
      stream << "\u2654";
      break;
    case Queen:
      stream << "\u2655";
      break;
    case Rook:
      stream << "\u2656";
      break;
    case Bishop:
      stream << "\u2657";
      break;
    case Knight:
      stream << "\u2658";
      break;
    case Pawn:
      stream << "\u2659";
      break;
    }
    // stream << std::format("\u{:x}", 9812 + piece.type);
    stream << "\033[0m";
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

    CreatePiece(White, Rook, 0);
    CreatePiece(White, Knight, 8);
    CreatePiece(White, Bishop, 16);
    CreatePiece(White, King, 24);
    CreatePiece(White, Queen, 32);
    CreatePiece(White, Bishop, 40);
    CreatePiece(White, Knight, 48);
    CreatePiece(White, Rook, 56);

    CreatePiece(White, Pawn, 1);
    CreatePiece(White, Pawn, 9);
    CreatePiece(White, Pawn, 17);
    CreatePiece(White, Pawn, 25);
    CreatePiece(White, Pawn, 33);
    CreatePiece(White, Pawn, 41);
    CreatePiece(White, Pawn, 49);
    CreatePiece(White, Pawn, 57);

    CreatePiece(Black, Pawn, 6);
    CreatePiece(Black, Pawn, 14);
    CreatePiece(Black, Pawn, 22);
    CreatePiece(Black, Pawn, 30);
    CreatePiece(Black, Pawn, 38);
    CreatePiece(Black, Pawn, 46);
    CreatePiece(Black, Pawn, 54);
    CreatePiece(Black, Pawn, 62);

    CreatePiece(Black, Rook, 7);
    CreatePiece(Black, Knight, 15);
    CreatePiece(Black, Bishop, 23);
    CreatePiece(Black, King, 31);
    CreatePiece(Black, Queen, 39);
    CreatePiece(Black, Bishop, 47);
    CreatePiece(Black, Knight, 55);
    CreatePiece(Black, Rook, 63);
  }

  void CreatePiece(Color color, PieceType type, int8_t square) {
    squares_[square] = pieces_.size();
    pieces_.push_back({color, type, square});
  }

  friend std::ostream &operator<<(std::ostream &stream, const Board &board) {
    for (int8_t rank = 0; rank < 8; ++rank) {
      for (int8_t file = 0; file < 8; ++file) {
        int8_t square = board.squares_[file * 8 + rank];
        if (square == -1) {
          stream << "\u23b5";
        } else {
          stream << board.pieces_[square];
        }
        stream << " ";
      }
      if (rank != 7) {
        stream << "\n";
      }
    }
    return stream;
  }

  // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
  // K for king, Q for queen, R for rook, B for bishop, and N for knight
  Move ParseAlgebraicNotation(std::string move) {
    // TODO do stripping if contains and x for capture or e.p. for en passant.
    // Also handle special case of castling

    PieceType piece_type;
    switch (move[0]) {
    case 'K':
      piece_type = King;
      break;
    case 'Q':
      piece_type = Queen;
      break;
    case 'R':
      piece_type = Rook;
      break;
    case 'B':
      piece_type = Bishop;
      break;
    case 'N':
      piece_type = Knight;
      break;
    default:
      piece_type = Pawn;
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
    case King:
      break;
    case Queen:
      break;
    case Rook:
      for (int i = 0; i < 8; ++i) {
        moves.push_back(8 * (piece.square / 8) + i); // file
        // if occupied, break
      }
      for (int i = 0; i < 8; ++i) {
        moves.push_back(8 * i + (piece.square % 8)); // rank
        // if occupied, break
      }
      break;
    case Bishop:
      break;
    case Knight:
      break;
    case Pawn:
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
  board.ParseAlgebraicNotation("Be5");
  return 0;
}
