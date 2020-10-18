/**
 *  @note This file is part of The Avida Digital Evolution Platform
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaWorld.h
 *  @brief This is the default, Avida-specific world.
 */

#include "AvidaOrg.h"

#include "emp/Evolve/World.h"
#include "emp/hardware/InstLib.h"

class AvidaWorld : public emp::World<AvidaOrg> {
public:
  using inst_lib_t = emp::AvidaCPU_InstLib<AvidaOrg>;

private:
  inst_lib_t inst_lib;

  double energy_threshold;  ///< Base energy requirement for an organism to replicate.
  double energy_inflow;     ///< Base amount of energy collected per update for each organism.

public:
  AvidaWorld() : inst_lib(AvidaOrg::DefaultInstLib()) {
    OnOrgPlacement( [this](size_t world_id){
      pop[world_id]->SetWorldID(world_id);  // Tell organisms their position in environment.
    } );
  }
  ~AvidaWorld() { ; }

  const inst_lib_t & GetInstLib() const { return inst_lib; }
  double GetEnergyThreshold() const { return energy_threshold; }
  double GetEnergyInflow() const { return energy_inflow; }

  bool DoReplicate(size_t repro_id) {
    emp_assert(IsOccupied(repro_id));
    AvidaOrg & org = GetOrg(repro_id);                     // Retrieve the organism to replicate.
    if (org.GetEnergy() < energy_threshold) return false;  // Not enough energy?  Stop!
    org.AdjustEnergy(-energy_threshold);                   // Pay the energy cost.
    DoBirth( org.GetGenome(), repro_id );                  // Trigger the birth.
    return true;
  }
};
