#include <iostream>
#include <string>

#include "../tourney_base.hpp"

// Prompts the user for command line input, which is parsed via `state.Parse`.
// If the parse fails, alerts the user and prompts for input again.
template <typename Move>
class HumanAgent final : public Agent<Move> {
 public:
  HumanAgent() = default;

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
