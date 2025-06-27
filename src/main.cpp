#include <iostream>

#include "agents/human_agent.hpp"
#include "agents/minimax_agent.hpp"
#include "games/chess.hpp"

int main() {
  // Set up the game and the agents
  auto game = Chess(/*white_perspective=*/true);
  auto human = HumanAgent<ChessMove>();
  auto minimax = MinimaxAgent<ChessMove>(/*max_plies=*/5);

  // Take turns making moves until someone can't
  while (!game.GetMoves().empty()) {
    std::cout << game.ToString() << "\n";
    const auto human_move = human.SelectMove(game);
    game.RecordMove(human_move);
    game.MakeMove(human_move);

    std::cout << game.ToString() << "\n";
    const auto minimax_move = minimax.SelectMove(game);
    game.RecordMove(minimax_move);
    game.MakeMove(minimax_move);
  }
  return 0;
}
