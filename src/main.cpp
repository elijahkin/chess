#include <cstdlib>
#include <iostream>
#include <string>

#include "games/chess.h"

int main() {
  Chess game = Chess(true);
  std::string input;
  while (true) {
    // Clear the screen and print out the board with history
    system("clear");
    std::cout << game.ToString() << "\n";

    // for (auto move : game.GetMoves()) {
    //   std::cout << game.ToString(move) << ' ';
    // }
    // std::cout << std::endl;

    // Loop until the user enters a valid move
    while (true) {
      std::cout << "Please enter a move: ";
      // TODO Overload >> instead of having a function?
      std::cin >> input;
      auto parsed = game.ParseAlgebraicNotation(input);
      if (parsed.has_value()) {
        game.MakeMakeWithHistory(parsed.value());
        break;
      }
      std::cout << "Invalid entry! ";
    }

    system("clear");
    std::cout << game.ToString() << "\n";

    std::cout << "Minimax agent is thinking...\n";
    Chess::Move black_move = game.Minimax(5);
    game.MakeMakeWithHistory(black_move);
  }
  return 0;
}
