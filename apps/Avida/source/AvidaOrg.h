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
  size_t world_id;
  double energy;

public:
  AvidaOrg() : world_id((size_t) -1), energy(0) { ; }
  AvidaOrg(const AvidaOrg &) = default;
  AvidaOrg(AvidaOrg &&) = default;

  size_t GetWorldID() const { return world_id; }
  double GetEnergy() const { return energy; }

  void SetWorldID(size_t _id) { world_id = _id; }
  void AdjustEnergy(double shift) { energy += shift; }

  static void Inst_Replicate(AvidaOrg & hw, const emp::AvidaGP::Instruction & inst) {
    hw.regs[inst.args[1]] = hw.regs[inst.args[0]];
  }
};
