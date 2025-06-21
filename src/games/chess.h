#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

// Define color scheme via ANSI escape codes
const std::string kPieceText = "\33[30m";
const std::string kHistoryText = "\33[38;5;240m";
const std::string kResetText = "\33[39m";
const std::string kWhiteBackground = "\33[47m";
const std::string kBlackBackground = "\33[45m";
const std::string kResetBackground = "\33[49m";

// https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
const std::vector<std::string> kUnicodePieces = {
    " ",      "\u2654", "\u2655", "\u2656", "\u2657", "\u2658", "\u2659",
    "\u265a", "\u265b", "\u265c", "\u265d", "\u265e", "\u265f"};

const std::vector<char> kPieceLetters = {'K', 'Q', 'R', 'B', 'N', '\0'};

enum Piece : int8_t {
  kEmpty,
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

const std::vector<int> kMaterialValues = {0,   40, 9,  5,  3,  3, 1,
                                          -40, -9, -5, -3, -3, -1};

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
  // This field determines from whose perspective we print the board
  bool white_perspective_;

 public:
  Chess(bool white_perspective) {
    // Define the types and order of pieces in white's major rank. The same
    // order is copied for black's major rank
    const std::vector<Piece> white_major = {
        kWhiteRook, kWhiteKnight, kWhiteBishop, kWhiteQueen,
        kWhiteKing, kWhiteBishop, kWhiteKnight, kWhiteRook};

    // Set up the board with the pieces in their starting positions. The ranks
    // are numbered starting from white's side of the board, such that white's
    // pieces start in ranks 1 and 2
    for (char rank = '1'; rank <= '8'; ++rank) {
      for (char file = 'a'; file <= 'h'; ++file) {
        Piece piece = kEmpty;
        if (rank == '1' || rank == '8') {
          piece = white_major[file - 'a'];
        } else if (rank == '2' || rank == '7') {
          piece = kWhitePawn;
        }
        if (rank == '7' || rank == '8') {
          piece = static_cast<Piece>(piece + 6);
        }
        SetPiece(file, rank, piece);
      }
    }
    white_to_move_ = true;
    white_perspective_ = white_perspective;
  }

  int8_t LogicalToPhysical(char file, char rank) const {
    return (8 * (rank - '1')) + (file - 'a');
  }

  void SetPiece(char file, char rank, Piece piece) {
    board_[LogicalToPhysical(file, rank)] = piece;
  }

  Piece GetPiece(char file, char rank) const {
    return board_[LogicalToPhysical(file, rank)];
  }

  friend std::ostream &operator<<(std::ostream &stream, const Chess &game) {
    // For each rank, print out the rank label on the left, then the squares of
    // that rank, then every eighth move in the move history
    std::string output;
    for (int row = 0; row < 9; ++row) {
      char rank = (game.white_perspective_ ? '8' - row : '1' + row);
      output += (row == 8 ? ' ' : rank);
      output += ' ';
      for (int col = 0; col < 8; ++col) {
        char file = (game.white_perspective_ ? 'a' + col : 'h' - col);
        if (row != 8) {
          // The board is such that top left square is white for both players
          output += ((row + col) % 2 ? kBlackBackground : kWhiteBackground);
          output += kPieceText;
          output += kUnicodePieces[game.GetPiece(file, rank)];
        } else {
          output += file;
        }
        output += ' ';
      }
      output += kResetBackground;
      // Print out every eighth move in the move history offset by rank
      output += kHistoryText;
      output += ' ';
      for (size_t move = row; move < game.history_.size(); move += 9) {
        // Accommodate for move numbers up to 999
        std::string move_str = std::to_string(move + 1);
        move_str.insert(0, 3 - move_str.size(), ' ');
        move_str += ". ";
        // Pad to support algebraic notation of different lengths
        move_str += game.history_[move];
        move_str.insert(move_str.end(), 10 - move_str.size(), ' ');
        output += move_str;
      }
      output += kResetText;
      output += '\n';
    }
    stream << output;
    return stream;
  }

  struct Move {
    int8_t from;
    int8_t to;
  };

  // Perform the move in memory and switch with the other player. We also return
  // the Piece previously on the square move.to in case we have to revert the
  // move later (such as during a minimax search)
  Piece MakeMove(Move move) {
    Piece captured = board_[move.to];

    board_[move.to] = board_[move.from];
    board_[move.from] = kEmpty;

    white_to_move_ = !white_to_move_;
    return captured;
  }

  void RevertMove(Move move, Piece captured) {
    board_[move.from] = board_[move.to];
    board_[move.to] = captured;
    white_to_move_ = !white_to_move_;
  }

  int MinimaxHelper(int8_t max_depth, int8_t depth, Move move) {
    // Make the move, storing any captured piece
    Piece captured = MakeMove(move);
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
    RevertMove(move, captured);
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
  // TODO supply this function based on which player is Minimax
  int MaterialAdvantage() const {
    int material_advantage = 0;
    for (auto piece : board_) {
      material_advantage -= kMaterialValues[piece];
    }
    return material_advantage;
  }

  // These terse function are designed to make the logic of scanning for moves
  // more intuitive. It is common to want to know whether a particular square is
  // occupied
  bool IsOccupied(int8_t square) const { return board_[square] != kEmpty; }

  // We would also often like to know whether a square is occupied by the
  // opponent
  bool IsOpponent(int8_t square) {
    return IsOccupied(square) && (IsWhite(board_[square]) != white_to_move_);
  }

  // Compute the legal moves for a bishop, rook, or queen on square 'from' can
  // move to on a chessboard. We can also compute the legal moves for a knight
  // and king by breaking on the first step
  std::vector<int8_t> MovesPattern(int8_t from,
                                   const std::vector<int8_t> step_sizes,
                                   bool knight_or_king = false) {
    std::vector<int8_t> tos;
    for (auto step_size : step_sizes) {
      for (int8_t i = 1;; ++i) {
        int8_t to = from + step_size * i;

        // Calculate the rank and file of the 'from' and 'to' squares
        int8_t from_rank = from / 8;
        int8_t from_file = from % 8;
        int8_t to_rank = to / 8;
        int8_t to_file = to % 8;

        // Ensure the move is within the board boundaries
        if (to < 0 || to >= 64) {
          break;
        }
        // Ensure the move does not wrap around the board
        // For horizontal moves, check if the file changes
        if ((step_size == -1 || step_size == 1) && to_rank != from_rank) {
          break;
        }
        // For vertical moves, check if the rank changes
        if ((step_size == -8 || step_size == 8) && to_file != from_file) {
          break;
        }
        // For diagonal moves, check if both rank and file change appropriately
        if ((abs(step_size) == 7 || abs(step_size) == 9) &&
            (abs(to_rank - from_rank) != i || abs(to_file - from_file) != i)) {
          break;
        }

        // We can move to a square if it is empty or has an opponent
        if (!IsOccupied(to) || IsOpponent(to)) {
          tos.push_back(to);
        }
        // We can't move through pieces
        if (IsOccupied(to) || knight_or_king) {
          break;
        }
      }
    }
    return tos;
  }

  // Compute a vector squares that a particular piece can move to. We use
  // fallthrough here for every piece type except pawns (pawns can only move in
  // one direction depending on color)
  std::vector<int8_t> LegalMoves(int8_t from) {
    switch (board_[from]) {
      case kEmpty:
        return {};
      case kWhiteKing:
      case kBlackKing:
        return MovesPattern(from, {-9, -8, -7, -1, 1, 7, 8, 9}, true);
      case kWhiteQueen:
      case kBlackQueen:
        return MovesPattern(from, {-9, -8, -7, -1, 1, 7, 8, 9});
      case kWhiteRook:
      case kBlackRook:
        return MovesPattern(from, {-8, -1, 1, 8});
      case kWhiteBishop:
      case kBlackBishop:
        return MovesPattern(from, {-9, -7, 7, 9});
      case kWhiteKnight:
      case kBlackKnight:
        return MovesPattern(from, {-17, -15, -10, -6, 6, 10, 15, 17}, true);
      case kWhitePawn:
      case kBlackPawn:
        // TODO Need to rewrite this; pawns are teleporting around the edges of
        // the board
        std::vector<int8_t> tos;
        int orientation = ((board_[from] == kWhitePawn) ? 1 : -1);
        int8_t forward = from + (8 * orientation);
        if (!IsOccupied(forward)) {
          tos.push_back(forward);
        }
        int8_t double_forward = from + (16 * orientation);
        if (!IsOccupied(double_forward) &&
            ((from / 8 == 1) || (from / 8 == 6))) {
          tos.push_back((double_forward));
        }
        int8_t left = from + (7 * orientation);
        if (IsOpponent(left)) {
          tos.push_back(left);
        }
        int8_t right = from + (9 * orientation);
        if (IsOpponent(right)) {
          tos.push_back(right);
        }
        return tos;
    }
  }

  std::vector<Move> LegalMoves() {
    std::vector<Move> moves;
    for (int8_t from = 0; from < 64; ++from) {
      if (IsWhite(board_[from]) == white_to_move_) {
        for (int8_t to : LegalMoves(from)) {
          moves.push_back({from, to});
        }
      }
    }
    return moves;
  }

  Move ParseAlgebraicNotation(std::string move) {
    // If the first letter is lowercase, this is a pawn move; We should insert
    // an extra character at the beginning
    if (std::islower(move[0])) {
      move.insert(0, " ");
    }

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
    const Piece type = static_cast<Piece>((6 * !white_to_move_) + offset);
    const int8_t to = LogicalToPhysical(move[1], move[2]);

    // Now we look for pieces of that type that can moved to the destination
    std::vector<int8_t> from_candidates;
    for (size_t from = 0; from < board_.size(); ++from) {
      if (board_[from] == type && IsWhite(board_[from]) == white_to_move_) {
        // TODO Is it viable?
        std::vector<int8_t> tos = LegalMoves(from);
        if (std::find(tos.begin(), tos.end(), to) != tos.end()) {
          from_candidates.push_back(from);
        }
      }
    }
    // The piece to move should now be unambiguous
    assert(from_candidates.size() == 1);
    return {from_candidates[0], to};
  }

  // Note that this function relies on the move not yet being made to retrieve
  // the piece type
  std::string ToString(Chess::Move move) {
    std::string output;
    output += kPieceLetters[(board_[move.from] - 1) % 6];
    output += (move.to % 8) + 'a';
    output += std::to_string(1 + (move.to / 8));
    return output;
  }

  // Log the move to history_ to be printed with the board
  void MakeMakeWithHistory(Move move) {
    history_.push_back(ToString(move));
    MakeMove(move);
  }
};
