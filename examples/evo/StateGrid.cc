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

  emp::evo::StateGrid sg(info, "test.sgrid");
  std::cout << "Created a " << sg.GetWidth() << "x" << sg.GetHeight()
	    << " state grid." << std::endl;

  sg.Write(std::cout);
}
