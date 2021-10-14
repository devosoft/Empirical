/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Mancala.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/games/Mancala.hpp"

TEST_CASE("Test Mancala", "[games]")
{
  emp::Mancala game;

  game.PrintSmall("Start");

  REQUIRE( game.IsDone() == false );

  //       (4) (4) (4) (4) (4) (4)         B <===
  //  (0)                          (0)
  //       (4) (4) (4) (4) (4) (4)         A ===>

  REQUIRE( game.GetA(0) == 4 );
  REQUIRE( game.GetA(4) == 4 );
  REQUIRE( game.GetA(6) == 0 );

  REQUIRE( game.GetB(1) == 4 );
  REQUIRE( game.GetB(3) == 4 );
  REQUIRE( game.GetB(6) == 0 );

  game.DoMove(3);
  game.PrintSmall("Move 3");

  //       (4) (4) (4) (4) (4) (5)
  //  (0)                          (1)
  //       (4) (4) (4) (0) (5) (5)

  REQUIRE( game.GetA(0) == 4 );
  REQUIRE( game.GetA(3) == 0 );
  REQUIRE( game.GetA(4) == 5 );
  REQUIRE( game.GetA(6) == 1 );

  REQUIRE( game.GetB(0) == 5 );
  REQUIRE( game.GetB(3) == 4 );
  REQUIRE( game.GetB(6) == 0 );

  game.DoMove(2);
  game.PrintSmall("Move 2");

  //       (5) (5) (5) (0) (4) (5)
  //  (1)                          (1)
  //       (4) (4) (4) (0) (5) (5)

  game.DoMove(5);  // Second move since previous ended at a goal.
  game.PrintSmall("Move 5");

  //       (0) (5) (5) (0) (4) (5)
  //  (2)                          (1)
  //       (5) (5) (5) (1) (5) (5)

  REQUIRE( game.GetA(0) == 5 );
  REQUIRE( game.GetA(3) == 1 );
  REQUIRE( game.GetA(4) == 5 );
  REQUIRE( game.GetA(6) == 1 );

  REQUIRE( game.GetB(0) == 5 );
  REQUIRE( game.GetB(3) == 5 );
  REQUIRE( game.GetB(6) == 2 );

  REQUIRE( game.IsDone() == false );
  REQUIRE( game.IsMoveValid(0) == true );
  REQUIRE( game.IsMoveValid(2) == true );
  REQUIRE( game.IsMoveValid(5) == true );

  game.DoMove(4);
  game.PrintSmall("Move 4");  // No capture since on OTHER side of the board.

  //       (0) (5) (5) (1) (5) (6)
  //  (2)                          (2)
  //       (5) (5) (5) (1) (0) (6)

  REQUIRE( game.GetA(0) == 5 );
  REQUIRE( game.GetA(3) == 1 );
  REQUIRE( game.GetA(4) == 0 );
  REQUIRE( game.GetA(6) == 2 );

  REQUIRE( game.GetB(0) == 6 );
  REQUIRE( game.GetB(2) == 1 );
  REQUIRE( game.GetB(3) == 5 );
  REQUIRE( game.GetB(6) == 2 );

  REQUIRE( game.IsDone() == false );
  REQUIRE( game.IsMoveValid(0) == true );
  REQUIRE( game.IsMoveValid(2) == true );
  REQUIRE( game.IsMoveValid(5) == false );

  auto options = game.GetMoveOptions();
  REQUIRE( options.size() == 5 );
  REQUIRE( options[4] == 4 );

  // Long string of moves by second player, ending in a capture
  game.DoMove(1);
  game.DoMove(5);
  game.DoMove(0);
  game.DoMove(5);
  game.DoMove(2);

  //       (1) (8) (8) (0) (1) (0)
  //  (11)                         (2)
  //       (0) (5) (5) (1) (0) (6)

  REQUIRE( game.GetA(0) == 0 );
  REQUIRE( game.GetA(3) == 1 );
  REQUIRE( game.GetA(4) == 0 );
  REQUIRE( game.GetA(6) == 2 );

  REQUIRE( game.GetB(0) == 0 );
  REQUIRE( game.GetB(2) == 0 );
  REQUIRE( game.GetB(3) == 8 );
  REQUIRE( game.GetB(6) == 11 );

  REQUIRE( game.IsDone() == false );
  REQUIRE( game.IsMoveValid(0) == false );
  REQUIRE( game.IsMoveValid(2) == true );
  REQUIRE( game.IsMoveValid(5) == true );

  REQUIRE (game.GetScore(0) == 19 );
  REQUIRE (game.GetScore(1) == 29 );

  game.Print();

  // Finish the game by having each player take their first move until it's done...
  while (game.IsDone() == false) {
    auto next = game.GetMoveOptions()[0];
    game.DoMove(next);
  }

  game.Print();

  // Test using board state as input.
  auto in_map = game.AsInput(0);
  auto in_vec = game.AsVectorInput(0);

  REQUIRE( in_map[0] == 0 );
  REQUIRE( in_map[1] == 0 );
  REQUIRE( in_map[2] == 0 );
  REQUIRE( in_map[6] == 19 );
  REQUIRE( in_map[7] == 0 );
  REQUIRE( in_map[8] == 6 );

  for (size_t i = 0; i < 14; i++) {
    REQUIRE( in_map[i] == in_vec[i] );
  }

  in_map = game.AsInput(1);
  in_vec = game.AsVectorInput(1);

  REQUIRE( in_map[0] == 0 );
  REQUIRE( in_map[1] == 6 );
  REQUIRE( in_map[2] == 0 );

  for (size_t i = 0; i < 14; i++) {
    REQUIRE( in_map[i] == in_vec[i] );
  }


  game.Reset();

  //       (4) (4) (4) (4) (4) (4)
  //  (0)                          (0)
  //       (4) (4) (4) (4) (4) (4)

  REQUIRE( game.GetA(0) == 4 );
  REQUIRE( game.GetA(4) == 4 );
  REQUIRE( game.GetA(6) == 0 );

  REQUIRE( game.GetB(1) == 4 );
  REQUIRE( game.GetB(3) == 4 );
  REQUIRE( game.GetB(6) == 0 );

}
