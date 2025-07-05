#include <array>
#include <iostream>
#include <memory>
#include <vector>

#include "agents/human_agent.hpp"
#include "agents/minimax_agent.hpp"
#include "games/chess.hpp"
#include "tourney_base.hpp"

constexpr auto kBlackAdvantageOnCapture = [](const ChessMove& move) {
  constexpr std::array<Score, 13> kMaterialValues = {
      0, 200, 9, 5, 3, 3, 1, -200, -9, -5, -3, -3, -1};
  return kMaterialValues[move.captured];
};

int main() {
  // Create the game and the agents playing it.
  auto game = Chess(/*white_perspective=*/true);

  std::vector<std::unique_ptr<Agent<ChessMove>>> agents;
  agents.push_back(std::make_unique<HumanAgent<ChessMove>>(game));
  agents.push_back(std::make_unique<MinimaxAgent<ChessMove>>(
      game, 5, kBlackAdvantageOnCapture));

  // Take turns making moves until someone can't.
  while (true) {
    for (const auto& agent_ptr : agents) {
      std::cout << game.ToString() << "\n";
      if (game.GenerateLegalMoves().empty()) {
        return 0;
      }
      const auto move = agent_ptr->SelectMove();
      game.RecordMove(move);
      game.MakeMove(move);
    }
  }
}
