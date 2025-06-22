#include <iostream>
#include <string>

#include "agents/human_agent.h"
#include "agents/minimax_agent.h"
#include "games/chess.h"

const std::string kClearTerminal = "\033[2J";
const std::string kCursorTopLeft = "\033[1;1H";

int main() {
  // Set up the game and the agents
  auto game = Chess(/*white_perspective=*/true);
  auto human = HumanAgent<ChessMove>();
  auto minimax = MinimaxAgent<ChessMove>(/*max_plies=*/5);

  while (true) {
    // Human player
    std::cout << kClearTerminal << kCursorTopLeft << game.ToString() << "\n";
    ChessMove white_move = human.SelectMove(game);
    game.RecordMove(white_move);
    game.MakeMove(white_move);

    // Minimax agent player
    std::cout << kClearTerminal << kCursorTopLeft << game.ToString() << "\n";
    std::cout << "Minimax agent is thinking...\n";
    ChessMove black_move = minimax.SelectMove(game);
    game.RecordMove(black_move);
    game.MakeMove(black_move);
  }
  return 0;
}
