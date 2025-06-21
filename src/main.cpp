#include <iostream>
#include <string>

#include "agents/minimax_agent.h"
#include "games/chess.h"

const std::string kClearTerminal = "\033[2J";
const std::string kCursorTopLeft = "\033[1;1H";

int main() {
  Chess game = Chess(/*white_perspective=*/true);

  auto minimax = MinimaxAgent<ChessMove>(5);

  while (true) {
    // Human player
    std::cout << kClearTerminal << kCursorTopLeft << game.ToString() << "\n";

    // Loop until the user enters a valid move
    std::string input;
    ChessMove white_move;
    while (true) {
      std::cout << "Please enter a move: ";
      std::cin >> input;
      auto parsed = game.ParseAlgebraicNotation(input);
      if (parsed.has_value()) {
        white_move = parsed.value();
        break;
      }
      std::cout << "Invalid entry! ";
    }
    game.MakeMakeWithHistory(white_move);

    // Minimax agent player
    std::cout << kClearTerminal << kCursorTopLeft << game.ToString() << "\n";

    std::cout << "Minimax agent is thinking...\n";
    ChessMove black_move = minimax.select_move(game);
    game.MakeMakeWithHistory(black_move);
  }
  return 0;
}
