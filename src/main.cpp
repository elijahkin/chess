#include <iostream>
#include <string>

#include "games/chess.h"

const std::string kClearTerminal = "\033[2J";
const std::string kCursorTopLeft = "\033[1;1H";

int main() {
  Chess game = Chess(/*white_perspective=*/true);

  while (true) {
    // Human player
    std::cout << kClearTerminal << kCursorTopLeft;
    std::cout << game.ToString() << "\n";

    // Loop until the user enters a valid move
    std::string input;
    while (true) {
      std::cout << "Please enter a move: ";
      std::cin >> input;
      auto parsed = game.ParseAlgebraicNotation(input);
      if (parsed.has_value()) {
        game.MakeMakeWithHistory(parsed.value());
        break;
      }
      std::cout << "Invalid entry! ";
    }

    // Minimax agent player
    std::cout << kClearTerminal << kCursorTopLeft;
    std::cout << game.ToString() << "\n";

    std::cout << "Minimax agent is thinking...\n";
    ChessMove black_move = game.Minimax(5);
    game.MakeMakeWithHistory(black_move);
  }
  return 0;
}
