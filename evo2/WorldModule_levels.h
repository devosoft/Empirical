//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A list of all available world modules.

#ifndef EMP_EVO_WORLD_MODULE_LEVELS_H
#define EMP_EVO_WORLD_MODULE_LEVELS_H

namespace emp {

  enum class evo {
    EA,       // Adds vector for next generation + changes "next" pointer to new vector

    Mixed,    // Well-mixed population structure.
    Grid,     // Grid population structure.
    Pools,    // Multiple pools population structure.

    Genotype, // Organisms have a genotype distinct from the organism. @CAO: Auto-detect?

    Environment, // Setup environmental resources (for use in fitness calculations.)

    CalcFit,  // Calculate fitness whenever it is needed.
    CacheFit, // Cache fitness for each organism.
    TrackFit, // Calculate all fitness at birth; track weighted ratios.

    Select,   // Add functions for selection methods.

    Stats,    // Add extra features to track what's going on in the world. @CAO: Multiple versions?
    Lineage,  // Calculate the full phylogeny as populations continue.

    Signals,  // Tie signals into the above functions to override them in a more dynamic way.

    UNKNOWN   // Unknown modifier; will trigger error.
  };

}

#endif
