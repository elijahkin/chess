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

typedef int8_t Square;

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
  Color colorToMove_;

public:
  // Set up the board with the pieces in their starting positions
  Board() {
    squares_.resize(64);
    for (int8_t i = 0; i < squares_.size(); ++i) {
      squares_[i] = -1;
    }

    pieces_.reserve(32);
    for (int8_t i = 0; i < 8; ++i) {
      CreatePiece(kBlack, major_rank[i], i);
      CreatePiece(kBlack, kPawn, i + 8);
      CreatePiece(kWhite, kPawn, i + 48);
      CreatePiece(kWhite, major_rank[i], i + 56);
    }

    colorToMove_ = kWhite;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Board &board) {
    stream << '\n';
    for (int8_t rank = 0; rank < 8; ++rank) {
      stream << static_cast<int>(8 - rank) << ' ';
      for (int8_t file = 0; file < 8; ++file) {
        stream << ((rank + file) % 2 ? kBlackBackground : kWhiteBackground);
        int8_t square = board.squares_[rank * 8 + file];
        stream << ' ';
        if (square == -1) {
          stream << ' ';
        } else {
          stream << board.pieces_[square];
        }
        stream << ' ';
      }
      stream << kResetBackground << '\n';
    }
    stream << "  ";
    for (char file = 'a'; file <= 'h'; ++file) {
      stream << ' ' << file << ' ';
    }
    return stream;
  }

  void CreatePiece(Color color, PieceType type, int8_t square) {
    squares_[square] = pieces_.size();
    pieces_.push_back({color, type, square});
  }

  struct Move {
    Square from;
    Square to;
  };

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

    std::vector<Square> candidates;
    for (auto piece : pieces_) {
      if (piece.type == piece_type && piece.color == colorToMove_) {
        // TODO Is it viable?
        candidates.push_back(piece.square);
      }
    }

    assert(candidates.size() == 1);
    const int8_t from = candidates[0];
    const int8_t to = (move[1] - 'a') + 8 * ('8' - move[2]);

    return {from, to};
  }

  // TODO std::vector<Square> getRank() {}
  // TODO std::vector<Square> getFile() {}
  // TODO std::vector<Square> getDiagonal() {}
  // TODO std::vector<Square> getAntiDiagonal() {}

  inline bool isOccupied(Square square) { return (squares_[square] == -1); }

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
    }
    return moves;
  }

  // TODO should verify that the move is legal
  inline void MakeMove(Square from, Square to) {
    // Update the square field of the captured and moving piece...
    // pieces_[squares_[to]].square = -1;
    pieces_[squares_[from]].square = to;
    // ...and update the indices stored in squares_
    squares_[to] = squares_[from];
    squares_[from] = -1;
    // Finally, switch to the other player
    colorToMove_ = ((colorToMove_ == kWhite) ? kBlack : kWhite);
  }
};

int main() {
  Board board;
  std::vector<std::string> history;
  std::string input;
  while (true) {
    // Clear the screen
    // TODO possibly move this into print
    system("clear");

    // Print out the board and the history
    // TODO move this into board print
    std::cout << board << "\n" << std::endl;
    for (int8_t move_number = 0; move_number < history.size(); ++move_number) {
      std::cout << (move_number + 1) << ". " << history[move_number] << " ";
    }
    std::cout << "\n" << std::endl;

    // Get use input for the move
    // TODO overload >> instead of having a function
    std::cout << "Please enter a move: ";
    std::cin >> input;
    history.push_back(input);
    Board::Move move = board.ParseAlgebraicNotation(input);

    board.MakeMove(move.from, move.to);
  }
  return 0;
}
