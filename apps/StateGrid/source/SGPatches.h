/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  SGPatches.h
 *  @brief A specialized StateGrid for handling resource patches.
 *
 *  This file manages StateGrids composed of resource patches in any topology (to be loaded).
 */

#ifndef STATE_GRID_PATCHES_H
#define STATE_GRID_PATCHES_H

#include "Evolve/StateGrid.h"

class SGPatches : public emp::StateGrid {
public:
  SGPatches() {
    AddState(-1, '-', -0.5, "None",          "Empty space; poisonous.");
    AddState( 0, '.',  0.0, "Consumed Food", "Previously had sustenance for an organism.");
    AddState( 1, '#', +1.0, "Food",          "sustenance to an org.");
    AddState( 2, 'x',  0.0, "Consumed Edge", "Edge marker; previously had food.");
    AddState( 3, 'X', +1.0, "Edge",          "Edge marker with food.");

    //Load("state_grids/islands_50x50.cfg");
    Load("state_grids/islands_50x50_paths.cfg");
  }

};

#endif
