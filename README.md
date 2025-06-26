# Tourney
Tourney is an open-source library for implementing games as well as artificial intelligence to play them. The fundamental classes are `Game` and `Agent`. A game (such as chess, go, etc.) can be added by deriving from the former and implementing each of its virtual functions. In contrast, agents are the objects which play the game; for example, the `HumanAgent` prompts the user for input, while `MinimaxAgent` searches the game tree for optimal moves.
## Introduction to Minimax
The [minimax algorithm](https://en.wikipedia.org/wiki/Minimax) is a well-known method of implementing a chess computer and the inspiration for this library. Essentially, the algorithm constructs a tree, where nodes denote board states and edges denote moves, of a certain depth. Note that this requires being able to enumerate all possible moves given some board state. Suppose I fix the depth of my tree to be 5; then the leaf nodes of the tree denote all possible board states 5 moves from now.

I then need to have some function that assigns a value to each leaf node. In chess, such a function could be the sum of my own material minus the sum of my opponent's material. We now arrive at the core idea of the algorithm -- we want to somehow deduce which move we should make *now*, given the values of all possible leaf nodes 5 moves from now. In particular, we want to maximize our value, knowing that our opponent while simultaneously be minimizing our value. This yields the minimax algorithm, where the value of a node is the maximum of its children's values at even depths, and the minimum of its children's values at odd depths.

Crucially, there is nothing about this algorithm that is specific to chess. In fact, it can be applied to a wide variety of two-player adversarial games. The original goal of Tourney was to make applying the minimax algorithm for any such game as simple as possible.
## Work in Progress
We divide work broadly into that which pertains specifically to `chess.hpp` and that which does not.
### Chess-specific Work
* Refactor to use [bitboard](https://en.wikipedia.org/wiki/Bitboard#Chess_bitboards) architecture. This will be tedious but (hopefully) worth the effort.
* Implement the [fifty-move rule](https://en.wikipedia.org/wiki/Fifty-move_rule).
* Implement special moves: pawn promotions, en passant capture, and castling.
* Add unit tests to guarantee correctness.
* Fix problem with knight move generation.
* Generalize parsing of algebraic notation to allow disambiguation via specification of the `from` square.
### General Work
* `HeuristicValue` should probably belong to `MinimaxAgent` rather than `Game`.
* `RecordAndMakeMove` should likely be another virtual function of `Game`.
* Implement `RandomAgent`, which selects uniformly from the set of possible moves.
* Design a tournament and ELO system to have many agents compete against each other.
* Implement other games: tic-tac-toe, dots and boxes, 2048, blackjack, and poker.
* Continuously rewrite code to make it as self-documenting as possible. Resort to comments sparingly. Ensure that "the function" is the implicit subject of all comments and that proper punctuation is used.
## References
* [Chess Programming Wiki](https://www.chessprogramming.org)
