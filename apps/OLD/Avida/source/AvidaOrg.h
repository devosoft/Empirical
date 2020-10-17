/**
 *  @note This file is part of The Avida Digital Evolution Platform
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaOrg.h
 *  @brief This is the default, Avida-specific organism.
 */

#ifndef AVIDA_ORG_H
#define AVIDA_ORG_H

#include "emp/hardware/AvidaCPU_InstLib.h"
#include "emp/hardware/AvidaGP.h"

class AvidaOrg : public emp::AvidaCPU_Base {
private:
  size_t world_id;
  double energy;

public:
  using this_t = AvidaOrg;
  using genome_t = typename emp::AvidaCPU_Base::Genome;
  using inst_lib_t = emp::AvidaCPU_InstLib<AvidaOrg>;

  AvidaOrg() : emp::AvidaCPU_Base(&(emp::DefaultInstLib())), world_id((size_t) -1), energy(0) { ; }
  AvidaOrg(const genome_t & genome)
    : emp::AvidaCPU_Base(genome), world_id((size_t) -1), energy(0) { ; }
  AvidaOrg(const AvidaOrg &) = default;
  AvidaOrg(AvidaOrg &&) = default;

  size_t GetWorldID() const { return world_id; }
  double GetEnergy() const { return energy; }

  void SetWorldID(size_t _id) { world_id = _id; }
  void AdjustEnergy(double shift) { energy += shift; }

  static void Inst_Replicate(AvidaOrg & hw, const emp::AvidaGP::Instruction & inst) {
    // hw.regs[inst.args[1]] = hw.regs[inst.args[0]];
  }

  static const inst_lib_t & DefaultInstLib() {
    static inst_lib_t inst_lib;

    if (inst_lib.GetSize() == 0) {
      inst_lib = inst_lib_t::DefaultInstLib();
      inst_lib.AddInst("Replicate", Inst_Replicate, 0, "Spend energy needed to reproduce Organism.");
    }

    return inst_lib;
  }

};

#endif
