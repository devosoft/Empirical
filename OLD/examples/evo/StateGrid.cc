//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides an example of using the StateGrid class.

#include "../../evo/StateGrid.h"

int main()
{
  emp::evo::StateGridInfo info;
  info.AddState(-1, '!', 0.9, "Poison", "Damages any organism that moves to this position.");
  info.AddState(0, '-', 1.0, "Empty", "Uninteresting position");
  info.AddState(1, '+', 1.1, "Food", "Moving to this position increases fitness (at least once.");

  emp::evo::StateGrid grid(info, "test.sgrid");
  std::cout << "Created a " << grid.GetWidth() << "x" << grid.GetHeight()
	    << " state grid." << std::endl;

  grid.Write(std::cout);

  emp::evo::StateGridStatus status;
  std::cout << "x=" << status.GetX() << " y=" << status.GetY() << " facing=" << status.GetFacing()
            << "  scan=" << status.Scan(grid) << std::endl;
  status.Move(grid, 4);
  std::cout << "x=" << status.GetX() << " y=" << status.GetY() << " facing=" << status.GetFacing()
            << "  scan=" << status.Scan(grid) << std::endl;
  status.Rotate();
  status.Move(grid, 2);
  std::cout << "x=" << status.GetX() << " y=" << status.GetY() << " facing=" << status.GetFacing()
            << "  scan=" << status.Scan(grid) << std::endl;
  status.Move(grid, 2);
  std::cout << "x=" << status.GetX() << " y=" << status.GetY() << " facing=" << status.GetFacing()
            << "  scan=" << status.Scan(grid) << std::endl;
}
