//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is an example file demonstrating Manacala.

#include <iostream>

#include "../../games/Othello.h"

int main()
{
  emp::Othello othello;
  size_t boardSize = 8;
  std::string gameNum = "";
  std::cout<< "Game Number "<<std::endl;
  std::cin >> gameNum;
  std::string filePath = "data/game_" + gameNum + ".csv";

  //std::cout << othello.GetSquare(4,4) <<std::endl;
  
  othello.Print();
  size_t moveX = 0;
  size_t moveY = 0;
  bool record = 0;
  int move_counter = 0;
  std::string input = "";
  othello.TestOver();
  std::ofstream myfile (filePath);

  while (othello.IsDone() == false) {
    //size_t player = mancala.GetCurPlayer();
    //char symbol = 'B' + (char) player;i
      if (othello.GetCurrPlayer() == 1) {std::cout << " X's "; }
      else { std::cout<<" O's "; }
      std::cout<<"Move? " << std::endl;
      emp::vector<std::pair<int, int>> validMoves = othello.GetMoveOptions(othello.GetCurrPlayer());
      for (auto el : validMoves) { std::cout<<el.first<< " " << el.second<<std::endl; }
      othello.ClearValidMoves();
      othello.ClearFlips();

      std::cin >> moveX >> moveY;
      /*std::getline(std::cin, input);
      if (!input.empty()) {record = 1;}*/
      if (move_counter == 10) {record = 1; }
      if (record) {
        othello.Write(&myfile, othello.GetCurrPlayer());
        
        }

      

      std::pair<int, int> move = std::make_pair(moveX, moveY);

    if (moveX < 1 || moveX > boardSize) {
      std::cout << "Invalid move!! (choose an X value 1 to " << boardSize<<")" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    if (moveY < 1 || moveY > boardSize) {
      std::cout << "Invalid move!! (choose an Y value 1 to " << boardSize<<")" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    

    if (othello.GetSquare(moveX, moveY) != 0) {
      std::cout << "Error: Cannot move to non-empty tile" << std::endl;
      continue;
    }

    if (othello.IsMoveValid(othello.GetCurrPlayer(), move ) == 0) {
        std::cout << "Invalid Move: Must flank at least one opponent disc" <<std::endl;
        continue;
    }

    bool tryMove = othello.DoMove(othello.GetCurrPlayer(), move);
    if (!tryMove) {othello.ChangeTurn();}
    move_counter++;
    othello.ClearFlips();

    othello.Print();

  }
  myfile.close();
  
  const size_t scoreB = othello.GetScore(1);
  const size_t scoreW = othello.GetScore(2);

  std::cout<<"Black: "<<scoreB<<" White: "<<scoreW<<std::endl<<std::endl;

  if (scoreW == scoreB) { std::cout << "TIE GAME!!" << std::endl; }
  else if (scoreW < scoreB) { std::cout << "Black Wins!!" << std::endl; }
  else { std::cout << "White Wins!!" << std::endl; }
}
