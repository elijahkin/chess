#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <regex>
#include <string>
#include <vector>

#include "../base.hpp"

// Assign human-readable names to ANSI escape codes.
constexpr std::string kCursorHome = "\x1B[H";
constexpr std::string kEraseScreen = "\x1B[2J";
constexpr std::string kForegroundBlack = "\x1B[30m";
constexpr std::string kForegroundGray = "\x1B[38;5;240m";
constexpr std::string kForegroundDefault = "\x1B[39m";
constexpr std::string kBackgroundMagenta = "\x1B[45m";
constexpr std::string kBackgroundWhite = "\x1B[47m";
constexpr std::string kBackgroundDefault = "\x1B[49m";

enum Piece : uint8_t {
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

using Square = uint8_t;

struct ChessMove {
  Square from;
  Square to;
  Piece captured;
};

class Chess final : public Game<ChessMove> {
 public:
  explicit Chess(bool white_perspective)
      : white_perspective_(white_perspective) {
    // Stores the types and order of pieces in white's major rank. Black's major
    // rank is deduced from this.
    const std::vector<Piece> white_major = {
        kWhiteRook, kWhiteKnight, kWhiteBishop, kWhiteQueen,
        kWhiteKing, kWhiteBishop, kWhiteKnight, kWhiteRook};

    // Set up the board with the pieces in their starting positions. The ranks
    // are numbered starting from white's side of the board, such that white's
    // pieces start in ranks 1 and 2.
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
  }

  // Performs the move in memory and changes to the other player's turn.
  void MakeMove(const ChessMove &move) override {
    board_[move.to] = board_[move.from];
    board_[move.from] = kEmpty;
    white_to_move_ = !white_to_move_;
  }

  void UnmakeMove(const ChessMove &move) override {
    board_[move.from] = board_[move.to];
    board_[move.to] = move.captured;
    white_to_move_ = !white_to_move_;
  }

  // Logs the move to `history_` for printing before making it.
  void RecordMove(const ChessMove &move) {
    if (white_to_move_) {
      history_.push_back(GetAlgebraicNotation(move));
    } else {
      std::string &tmp = history_.back();
      tmp.insert(tmp.length(), " ");
      tmp.insert(tmp.length(), GetAlgebraicNotation(move));
    }
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
    // For each rank, prints out the rank label on the left, then the squares of
    // that rank, then every ninth move in the move history.
    std::string output = kEraseScreen + kCursorHome;
    for (size_t row = 0; row < 9; ++row) {
      const char rank =
          static_cast<char>(white_perspective_ ? '8' - row : '1' + row);
      output += (row == 8 ? ' ' : rank);
      output += ' ';
      for (size_t col = 0; col < 8; ++col) {
        const char file =
            static_cast<char>(white_perspective_ ? 'a' + col : 'h' - col);
        if (row != 8) {
          // The board is such that top left square is white for both players
          output +=
              ((row + col) % 2 == 0) ? kBackgroundWhite : kBackgroundMagenta;
          output += kForegroundBlack;
          output += kUnicodePieces[board_[LogicalToPhysical(file, rank)]];
        } else {
          output += file;
        }
        output += ' ';
      }
      output += kBackgroundDefault;
      // Prints out every ninth move in the move history offset by rank.
      output += kForegroundGray;
      output += ' ';
      for (size_t move = row; move < history_.size(); move += 9) {
        // Accommodates move numbers up to 999.
        std::string move_str = std::to_string(move + 1);
        move_str.insert(0, 3 - move_str.size(), ' ');
        move_str += ". ";
        // Pads to support algebraic notation of different lengths.
        move_str += history_[move];
        move_str.insert(move_str.end(), 14 - move_str.size(), ' ');
        output += move_str;
      }
      output += kForegroundDefault;
      output += '\n';
    }
    return output;
  }

  // Parses user-input algebraic notation.
  [[nodiscard]] std::optional<ChessMove> Parse(
      const std::string &move) const override {
    const std::regex pattern("([BKNRQ]?)x?([a-h])([1-8])");

    // Ensures syntactic correctness of input.
    std::smatch matches;
    if (!std::regex_match(move, matches, pattern)) {
      return std::nullopt;
    }

    // Computes the piece type and destination from the captured groups.
    const char piece = matches[1].str()[0];
    const char to_file = matches[2].str()[0];
    const char to_rank = matches[3].str()[0];
    int8_t offset = 6;
    switch (piece) {
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
        break;
    }
    const auto type = static_cast<Piece>(offset + (6 * !white_to_move_));
    const Square to = LogicalToPhysical(to_file, to_rank);

    // Seaches for pieces of that type that can moved to the destination.
    std::vector<Square> from_candidates;
    for (Square from = 0; from < 64; ++from) {
      if (board_[from] == type && IsWhite(board_[from]) == white_to_move_) {
        std::vector<Square> tos = GetToSquares(from);
        if (std::ranges::find(tos, to) != tos.end()) {
          from_candidates.push_back(from);
        }
      }
    }
    // Abandons the parse if there is not exactly one such piece.
    if (from_candidates.size() != 1) {
      return std::nullopt;
    }
    return ChessMove{
        .from = from_candidates[0], .to = to, .captured = board_[to]};
  }

 private:
  // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
  static constexpr std::array<std::string, 13> kUnicodePieces = {
      " ",      "\u2654", "\u2655", "\u2656", "\u2657", "\u2658", "\u2659",
      "\u265a", "\u265b", "\u265c", "\u265d", "\u265e", "\u265f"};

  static constexpr std::array<char, 6> kPieceLetters = {'K', 'Q', 'R',
                                                        'B', 'N', '\0'};

  // Converts the rank and file on the chess board to the index of the
  // corresponding square in `board_`.
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

  // Computes the legal moves for sliding pieces (bishop, rook, or queen) on
  // square `from`. Can also compute the legal moves for knight and king by
  // breaking after the first step.
  void InsertToSquaresSliding(Square from, std::vector<Square> &tos,
                              const std::vector<int8_t> &pattern,
                              bool knight_or_king = false) const {
    const auto from_rank = static_cast<int8_t>(from / 8);
    const auto from_file = static_cast<int8_t>(from % 8);

    for (auto step_size : pattern) {
      for (int8_t i = 1;; ++i) {
        const auto to = static_cast<Square>(from + (step_size * i));

        // Ensures the move is within the board boundaries. Since `Square` is an
        // alias for `uint8_t`, checking for negativity is not necessary.
        if (to >= 64) {
          break;
        }

        const auto to_rank = static_cast<int8_t>(to / 8);
        const auto to_file = static_cast<int8_t>(to % 8);

        if (abs(step_size) == 1 && to_rank != from_rank) {
          // Prevents horizontal moves from changing the rank.
          break;
        }
        if (abs(step_size) == 8 && to_file != from_file) {
          // Prevents vertical moves from changing the file.
          break;
        }
        if ((abs(step_size) == 7 || abs(step_size) == 9) &&
            (abs(to_rank - from_rank) != i || abs(to_file - from_file) != i)) {
          // For diagonal moves, checks if rank and file change equally.
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
      const auto capture_left = static_cast<Square>(from + (7 * orientation));
      if (IsOpponent(capture_left)) {
        tos.push_back(capture_left);
      }
    }
    if ((from_file != 7 || board_[from] != kWhitePawn) &&
        (from_file != 0 || board_[from] != kBlackPawn)) {
      const auto capture_right = static_cast<Square>(from + (9 * orientation));
      if (IsOpponent(capture_right)) {
        tos.push_back(capture_right);
      }
    }
  }

  // Computes a vector of squares that the piece at `from` can move to.
  [[nodiscard]] std::vector<Square> GetToSquares(Square from) const {
    std::vector<Square> tos;
    switch (board_[from]) {
      case kEmpty:
        break;
      case kWhiteKing:
      case kBlackKing:
        InsertToSquaresSliding(from, tos, {-9, -8, -7, -1, 1, 7, 8, 9}, true);
        break;
      case kWhiteQueen:
      case kBlackQueen:
        InsertToSquaresSliding(from, tos, {-9, -8, -7, -1, 1, 7, 8, 9});
        break;
      case kWhiteRook:
      case kBlackRook:
        InsertToSquaresSliding(from, tos, {-8, -1, 1, 8});
        break;
      case kWhiteBishop:
      case kBlackBishop:
        InsertToSquaresSliding(from, tos, {-9, -7, 7, 9});
        break;
      case kWhiteKnight:
      case kBlackKnight:
        InsertToSquaresSliding(from, tos, {-17, -15, -10, -6, 6, 10, 15, 17},
                               true);
        break;
      case kWhitePawn:
      case kBlackPawn:
        InsertToSquaresPawn(from, tos);
        break;
    }
    return tos;
  }

  // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
  [[nodiscard]] std::string GetAlgebraicNotation(const ChessMove &move) const {
    std::string output;
    output += kPieceLetters[(board_[move.from] - 1) % 6];
    if (move.captured != kEmpty) {
      output += 'x';
    }
    output += (move.to % 8) + 'a';
    output += std::to_string(1 + (move.to / 8));
    return output;
  }

  // Stores the board rank-major such that the squares laid out in `board_` like
  // so: 1a ... 1h 2a ... 2h ... 7h 8a ... 8h.
  std::array<Piece, 64> board_{};

  // Records the move history in algebraic notation.
  std::vector<std::string> history_;

  // Keeps track of whose turn it is.
  bool white_to_move_ = true;

  // Determines from whose perspective we print the board.
  bool white_perspective_;
};
