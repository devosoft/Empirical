#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/games/Othello.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"

TEST_CASE("Test Othello", "[games]") {
  constexpr size_t board_width_w8 = 8;
  constexpr size_t board_width_w1024 = 1024;

  emp::Othello othello_w8;
  emp::Othello_Game<board_width_w1024> othello_w1024;

  emp::Random random(2);

  // Check board sizes.
  REQUIRE(othello_w8.GetNumCells() == 8*8);
  REQUIRE(othello_w1024.GetNumCells() == 1024*1024);

  // Check get neighbors function for 0,0
  // Illegal directions.
  int eNid = -1, eNEid = -1, eSWid = -1, eWid = -1, eNWid = -1;
  // Legal directions.
  int eSid = 8, eSEid = 9, eEid = 1;
  // What does othello say the neighbors are for 0,0?
  auto Nid = othello_w8.GetNeighbor(0, emp::Othello::N);
  auto NEid = othello_w8.GetNeighbor(0, emp::Othello::NE);
  auto Eid = othello_w8.GetNeighbor(0, emp::Othello::E);
  auto SEid = othello_w8.GetNeighbor(0, emp::Othello::SE);
  auto Sid = othello_w8.GetNeighbor(0, emp::Othello::S);
  auto SWid = othello_w8.GetNeighbor(0, emp::Othello::SW);
  auto Wid = othello_w8.GetNeighbor(0, emp::Othello::W);
  auto NWid = othello_w8.GetNeighbor(0, emp::Othello::NW);
  REQUIRE(Nid.IsValid() == false);
  REQUIRE(NEid.IsValid() == false);
  REQUIRE(Eid == 1);
  REQUIRE(SEid == 9);
  REQUIRE(Sid == 8);
  REQUIRE(SWid.IsValid() == false);
  REQUIRE(Wid.IsValid() == false);
  REQUIRE(NWid.IsValid() == false);

  // On initial board:
  // - Make sure all positions we expect to be valid are valid.
  auto player_dark = emp::Othello::Player::DARK;
  auto player_light = emp::Othello::Player::LIGHT;
  REQUIRE(othello_w8.IsValidMove(player_light, {4,2}));
  REQUIRE(othello_w8.IsValidMove(player_light, {2,4}));
  REQUIRE(othello_w8.IsValidMove(player_light, {5,3}));
  REQUIRE(othello_w8.IsValidMove(player_light, {3,5}));
  REQUIRE(othello_w8.IsValidMove(player_dark, {3,2}));
  REQUIRE(othello_w8.IsValidMove(player_dark, {2,3}));
  REQUIRE(othello_w8.IsValidMove(player_dark, {5,4}));
  REQUIRE(othello_w8.IsValidMove(player_dark, {4,5}));
  // - And all positions are owned by who we expect.
  auto l0id = othello_w8.GetIndex(3,3); // light
  auto d0id = othello_w8.GetIndex(3,4); // dark
  auto d1id = othello_w8.GetIndex(4,3);  // dark
  auto l1id = othello_w8.GetIndex(4,4); // light
  REQUIRE(othello_w8.GetPosOwner(l0id) == player_light);
  REQUIRE(othello_w8.GetPosOwner(d0id) == player_dark);
  REQUIRE(othello_w8.GetPosOwner(d1id) == player_dark);
  REQUIRE(othello_w8.GetPosOwner(l1id) == player_light);
  // Check initial board setup.
  // -- Score should be 2,2.
  REQUIRE(othello_w8.GetScore(player_dark) == 2);
  REQUIRE(othello_w8.GetScore(player_light) == 2);
  REQUIRE(othello_w1024.GetScore(player_dark) == 2);
  REQUIRE(othello_w1024.GetScore(player_light) == 2);
  // -- Check get Frontier position count.
  REQUIRE(othello_w8.CountFrontierPos(player_dark) == 10);
  REQUIRE(othello_w8.CountFrontierPos(player_light) == 10);
  // -- Check get flip list for 1 valid move.
  REQUIRE(othello_w8.GetFlipList(player_dark, othello_w8.GetIndex(3,2)).size() == 1);

  // Make some moves. Check that they altered the board properly.
  othello_w8.DoMove(player_dark, {3,2});
  REQUIRE(othello_w8.GetPosOwner({3,2}) == player_dark);
  REQUIRE(othello_w8.GetPosOwner({3,3}) == player_dark);
  REQUIRE(othello_w8.GetScore(player_dark) == 4);
  REQUIRE(othello_w8.GetScore(player_light) == 1);
  REQUIRE(othello_w8.GetCurPlayer() == player_light);

  // Play the rest of the game randomly a bunch of times.
  size_t rounds;
  for (size_t i = 0; i < 100; ++i) {
    othello_w8.Reset();
    rounds = 0;
    while (!othello_w8.IsOver()) {
      REQUIRE(rounds <= 60);  //< Max of 60 rounds possible.
      auto player = othello_w8.GetCurPlayer();
      auto options = othello_w8.GetMoveOptions(player);
      auto move = options[random.GetUInt(0, options.size())];
      othello_w8.DoMove(player, move);
      rounds++;
    }
  }
}
