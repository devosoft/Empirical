/**
 *  @note This file is part of The Avida Digital Evolution Platform
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaOrg.h
 *  @brief This is the default, Avida-specific organism.
 */

 #include "hardware/AvidaGP.h"

class AvidaOrg : public emp::AvidaGP {
private:
  double energy;
public:
  AvidaOrg() : energy(0) { ; }

  double GetEnergy() const { return energy; }

  void AdjustEnergy(double shift) { energy += shift; }
};
