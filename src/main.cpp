#include <iostream>

#include "agents/human_agent.h"
#include "agents/minimax_agent.h"
#include "games/chess.h"

int main() {
  // Set up the game and the agents
  auto game = Chess(/*white_perspective=*/true);
  auto human = HumanAgent<ChessMove>();
  auto minimax = MinimaxAgent<ChessMove>(/*max_plies=*/5);

  while (true) {
    // Human player
    std::cout << game.ToString() << "\n";
    const auto human_move = human.SelectMove(game);
    game.RecordMove(human_move);
    game.MakeMove(human_move);

    // Minimax agent player
    std::cout << game.ToString() << "\n";
    const auto minimax_move = minimax.SelectMove(game);
    game.RecordMove(minimax_move);
    game.MakeMove(minimax_move);
  }
  return 0;
}
