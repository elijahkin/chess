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

    // for (auto move : game.LegalMoves()) {
    //   std::cout << game.ToString(move) << ' ';
    // }
    // std::cout << std::endl;

    // Allow the user to input a move
    std::cout << "Please enter a move: ";
    // TODO Overload >> instead of having a function?
    std::cin >> input;
    Chess::Move white_move = game.ParseAlgebraicNotation(input);
    game.MakeMakeWithHistory(white_move);

    system("clear");
    std::cout << game.ToString() << "\n";

    Chess::Move black_move = game.Minimax(5);
    game.MakeMakeWithHistory(black_move);
  }
  return 0;
}
