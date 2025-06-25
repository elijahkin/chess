#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

#include "../base.h"

// Define color scheme via ANSI escape codes
const std::string kClearTerminal = "\033[2J";
const std::string kCursorTopLeft = "\033[1;1H";
const std::string kPieceText = "\33[30m";
const std::string kHistoryText = "\33[38;5;240m";
const std::string kResetText = "\33[39m";
const std::string kWhiteBackground = "\33[47m";
const std::string kBlackBackground = "\33[45m";
const std::string kResetBackground = "\33[49m";

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

// https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
const std::vector<std::string> kUnicodePieces = {
    " ",      "\u2654", "\u2655", "\u2656", "\u2657", "\u2658", "\u2659",
    "\u265a", "\u265b", "\u265c", "\u265d", "\u265e", "\u265f"};

const std::vector<char> kPieceLetters = {'K', 'Q', 'R', 'B', 'N', '\0'};

const std::vector<int> kMaterialValues = {0,   40, 9,  5,  3,  3, 1,
                                          -40, -9, -5, -3, -3, -1};

using Square = int8_t;

struct ChessMove {
  Square from;
  Square to;
  Piece captured;
};

class Chess : public Game<ChessMove> {
 public:
  explicit Chess(bool white_perspective) {
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
        board_[LogicalToPhysical(file, rank)] = piece;
      }
    }
    white_to_move_ = true;
    white_perspective_ = white_perspective;
  }

  // Compute the sum of white's material minus the sum of black's material
  // TODO supply this function based on which player is Minimax
  [[nodiscard]] double Valuate() const override {
    double material_advantage = 0;
    for (auto piece : board_) {
      material_advantage -= kMaterialValues[piece];
    }
    return material_advantage;
  }

  // Perform the move in memory and switch with the other player. We also return
  // the Piece previously on the square move.to in case we have to revert the
  // move later (such as during a minimax search)
  void MakeMove(const ChessMove &move) override {
    board_[move.to] = board_[move.from];
    board_[move.from] = kEmpty;
    white_to_move_ = !white_to_move_;
  }

  void RevertMove(const ChessMove &move) override {
    board_[move.from] = board_[move.to];
    board_[move.to] = move.captured;
    white_to_move_ = !white_to_move_;
  }

  [[nodiscard]] std::vector<ChessMove> GetMoves() const override {
    std::vector<ChessMove> moves;
    for (Square from = 0; from < 64; ++from) {
      if (IsWhite(board_[from]) == white_to_move_) {
        for (Square to : GetToSquares(from)) {
          moves.push_back({from, to, board_[to]});
        }
      }
    }
    return moves;
  }

  [[nodiscard]] std::string ToString() const override {
    // For each rank, print out the rank label on the left, then the squares of
    // that rank, then every eighth move in the move history
    std::string output = kClearTerminal + kCursorTopLeft;
    for (int row = 0; row < 9; ++row) {
      const char rank =
          static_cast<char>(white_perspective_ ? '8' - row : '1' + row);
      output += (row == 8 ? ' ' : rank);
      output += ' ';
      for (int col = 0; col < 8; ++col) {
        const char file =
            static_cast<char>(white_perspective_ ? 'a' + col : 'h' - col);
        if (row != 8) {
          // The board is such that top left square is white for both players
          output +=
              ((row + col) % 2 == 0) ? kWhiteBackground : kBlackBackground;
          output += kPieceText;
          output += kUnicodePieces[board_[LogicalToPhysical(file, rank)]];
        } else {
          output += file;
        }
        output += ' ';
      }
      output += kResetBackground;
      // Print out every eighth move in the move history offset by rank
      output += kHistoryText;
      output += ' ';
      for (size_t move = row; move < history_.size(); move += 9) {
        // Accommodate for move numbers up to 999
        std::string move_str = std::to_string(move + 1);
        move_str.insert(0, 3 - move_str.size(), ' ');
        move_str += ". ";
        // Pad to support algebraic notation of different lengths
        move_str += history_[move];
        move_str.insert(move_str.end(), 14 - move_str.size(), ' ');
        output += move_str;
      }
      output += kResetText;
      output += '\n';
    }
    return output;
  }

  // Parse user-input algebraic chess notation
  [[nodiscard]] std::optional<ChessMove> Parse(
      std::string move) const override {
    // If the first letter is lowercase, this is a pawn move; We should insert
    // an extra character at the beginning
    if (std::islower(move[0]) != 0) {
      move.insert(0, " ");
    }

    // TODO Strip out x for capture or e.p. for en passant.
    // TODO Also handle special case of castling

    // Ensure string refers to a valid square on the board
    if (move[1] < 'a' || move[1] > 'h' || move[2] < '1' || move[2] > '8') {
      return std::nullopt;
    }

    // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
    // Read off the type of the piece being moved and its destination
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
    const auto type = static_cast<Piece>(offset + (6 * !white_to_move_));
    const Square to = LogicalToPhysical(move[1], move[2]);

    // Now we look for pieces of that type that can moved to the destination
    std::vector<Square> from_candidates;
    for (Square from = 0; from < 64; ++from) {
      if (board_[from] == type && IsWhite(board_[from]) == white_to_move_) {
        std::vector<Square> tos = GetToSquares(from);
        if (std::ranges::find(tos, to) != tos.end()) {
          from_candidates.push_back(from);
        }
      }
    }
    // The piece to move should be unambiguous
    if (from_candidates.size() != 1) {
      return std::nullopt;
    }
    return ChessMove{
        .from = from_candidates[0], .to = to, .captured = board_[to]};
  }

  // Log the move to `history_` to be printed with the board
  void RecordMove(const ChessMove &move) {
    if (white_to_move_) {
      history_.push_back(ToString(move));
    } else {
      std::string &tmp = history_.back();
      tmp.insert(tmp.length(), " ");
      tmp.insert(tmp.length(), ToString(move));
    }
  }

 private:
  // Converts the rank and file on the chess board to the index of the
  // corresponding square in `board_`
  static Square LogicalToPhysical(char file, char rank) {
    return (8 * (rank - '1')) + (file - 'a');
  }

  static bool IsWhite(const Piece piece) {
    return (piece >= kWhiteKing && piece <= kWhitePawn);
  }

  [[nodiscard]] bool IsOccupied(Square square) const {
    return board_[square] != kEmpty;
  }

  [[nodiscard]] bool IsOpponent(Square square) const {
    return IsOccupied(square) && (IsWhite(board_[square]) != white_to_move_);
  }

  // Compute the legal moves for a bishop, rook, or queen on square `from` can
  // move to on a chessboard. We can also compute the legal moves for a knight
  // and king by breaking on the first step
  void InsertToSquaresPattern(Square from, std::vector<Square> &tos,
                              const std::vector<int8_t> &step_sizes,
                              bool knight_or_king = false) const {
    // Calculate the rank and file of the `from` square
    const auto from_rank = static_cast<int8_t>(from / 8);
    const auto from_file = static_cast<int8_t>(from % 8);

    for (auto step_size : step_sizes) {
      for (int8_t i = 1;; ++i) {
        const auto to = static_cast<Square>(from + (step_size * i));

        // Ensure the move is within the board boundaries
        if (to < 0 || to >= 64) {
          break;
        }

        // Calculate the rank and file of the `to` square
        const auto to_rank = static_cast<int8_t>(to / 8);
        const auto to_file = static_cast<int8_t>(to % 8);

        // Ensure the move does not wrap around the board
        if (abs(step_size) == 1 && to_rank != from_rank) {
          // For horizontal moves, check if the file changes
          break;
        }
        if (abs(step_size) == 8 && to_file != from_file) {
          // For vertical moves, check if the rank changes
          break;
        }
        if ((abs(step_size) == 7 || abs(step_size) == 9) &&
            (abs(to_rank - from_rank) != i || abs(to_file - from_file) != i)) {
          // For diagonal moves, check if rank and file change equally
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
  }

  void InsertToSquaresPawn(Square from, std::vector<Square> &tos) const {
    // Calculate the rank and file of the `from` square
    const auto from_rank = static_cast<int8_t>(from / 8);
    const auto from_file = static_cast<int8_t>(from % 8);

    const int orientation = (board_[from] == kWhitePawn) ? 1 : -1;

    if ((from_rank != 7 || board_[from] != kWhitePawn) &&
        (from_rank != 0 || board_[from] != kBlackPawn)) {
      const auto forward = static_cast<Square>(from + (8 * orientation));
      if (!IsOccupied(forward)) {
        tos.push_back(forward);

        if ((from_rank == 1 && board_[from] == kWhitePawn) ||
            (from_rank == 6 && board_[from] == kBlackPawn)) {
          const auto double_forward =
              static_cast<Square>(from + (16 * orientation));
          if (!IsOccupied(double_forward)) {
            tos.push_back((double_forward));
          }
        }
      }
    }

    if ((from_file != 0 || board_[from] != kWhitePawn) &&
        (from_file != 7 || board_[from] != kBlackPawn)) {
      const auto attack_left = static_cast<Square>(from + (7 * orientation));
      if (IsOpponent(attack_left)) {
        tos.push_back(attack_left);
      }
    }
    if ((from_file != 7 || board_[from] != kWhitePawn) &&
        (from_file != 0 || board_[from] != kBlackPawn)) {
      const auto attack_right = static_cast<Square>(from + (9 * orientation));
      if (IsOpponent(attack_right)) {
        tos.push_back(attack_right);
      }
    }
  }

  // Computes a vector of squares that the piece at `from` can move to
  [[nodiscard]] std::vector<Square> GetToSquares(Square from) const {
    std::vector<Square> tos;
    switch (board_[from]) {
      case kEmpty:
        // TODO If we ever reach this, we're wasting function calls
        break;
      case kWhiteKing:
      case kBlackKing:
        InsertToSquaresPattern(from, tos, {-9, -8, -7, -1, 1, 7, 8, 9}, true);
        break;
      case kWhiteQueen:
      case kBlackQueen:
        InsertToSquaresPattern(from, tos, {-9, -8, -7, -1, 1, 7, 8, 9});
        break;
      case kWhiteRook:
      case kBlackRook:
        InsertToSquaresPattern(from, tos, {-8, -1, 1, 8});
        break;
      case kWhiteBishop:
      case kBlackBishop:
        InsertToSquaresPattern(from, tos, {-9, -7, 7, 9});
        break;
      case kWhiteKnight:
      case kBlackKnight:
        InsertToSquaresPattern(from, tos, {-17, -15, -10, -6, 6, 10, 15, 17},
                               true);
        break;
      case kWhitePawn:
      case kBlackPawn:
        InsertToSquaresPawn(from, tos);
        break;
    }
    return tos;
  }

  // Note that this function relies on the move not yet being made to retrieve
  // the piece type
  [[nodiscard]] std::string ToString(const ChessMove &move) const {
    std::string output;
    output += kPieceLetters[(board_[move.from] - 1) % 6];
    output += (move.to % 8) + 'a';
    output += std::to_string(1 + (move.to / 8));
    return output;
  }

  // In memory, we store the board rank-major such that the squares laid out in
  // board_ like so: 1a ... 1h 2a ... 2h ... 7h 8a ... 8h. This makes it easier
  // to initialize the board in its starting position
  std::array<Piece, 64> board_{};

  // Keeps track of whose turn it is
  bool white_to_move_;

  // We record the algebraic notation of each move in this vector
  std::vector<std::string> history_;

  // Determines from whose perspective we print the board
  bool white_perspective_;
};
