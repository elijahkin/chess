#include <iostream>
#include <string>

#include "../base.h"

template <typename Move>
class HumanAgent : public Agent<Move> {
 public:
  HumanAgent() = default;

  // Loop until the user enters a valid move
  Move SelectMove(Game<Move> &state) override {
    std::string input;
    while (true) {
      std::cout << "Please enter a move: ";
      std::cin >> input;
      auto parsed = state.Parse(input);
      if (parsed.has_value()) {
        return parsed.value();
        break;
      }
      std::cout << "Invalid entry! ";
    }
  }
};
