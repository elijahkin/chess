#include <iostream>
#include <memory>
#include <vector>

#include "agents/human_agent.hpp"
#include "agents/minimax_agent.hpp"
#include "base.hpp"
#include "games/chess.hpp"

int main() {
  // Create the game and the agents playing it.
  auto game = Chess(/*white_perspective=*/true);

  std::vector<std::unique_ptr<Agent<ChessMove>>> agents;
  agents.push_back(std::make_unique<HumanAgent<ChessMove>>());
  agents.push_back(std::make_unique<MinimaxAgent<ChessMove>>(/*max_plies=*/5));

  // Take turns making moves until someone can't.
  while (true) {
    for (const auto& agent_ptr : agents) {
      std::cout << game.ToString() << "\n";
      if (game.GetMoves().empty()) {
        return 0;
      }
      const auto move = agent_ptr->SelectMove(game);
      game.RecordMove(move);
      game.MakeMove(move);
    }
  }
}
