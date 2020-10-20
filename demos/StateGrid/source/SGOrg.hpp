/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  SGOrg.h
 *  @brief An Avida-inspired organism that can navigate a state grid.
 *
 *  This file manages StateGrid Organisms that can scan the current posiiton, move forward,
 *  or rotate.
 */

#include "Evolve/StateGrid.h"
#include "hardware/AvidaGP.h"
#include "hardware/InstLib.h"

class SGOrg : public emp::AvidaCPU_Base<SGOrg> {
 protected:
  emp::StateGridStatus sg_status;
  emp::StateGrid state_grid;
  double score;

 public:
  using base_t = emp::AvidaCPU_Base<SGOrg>;

 SGOrg() : sg_status(), state_grid(), score(0) { ; }
  SGOrg(emp::Ptr< const emp::AvidaCPU_InstLib<SGOrg> > inst_lib)
    : base_t(inst_lib), sg_status(), state_grid(), score(0) { ; }
 SGOrg(const base_t::genome_t & in_genome) : base_t(in_genome), sg_status(), state_grid(), score(0) { ; }
  SGOrg(const SGOrg &) = default;
  SGOrg(SGOrg &&) = default;

  emp::StateGridStatus & GetSGStatus() { return sg_status; }
  emp::StateGridStatus GetSGStatus() const { return sg_status; }
  emp::StateGrid & GetStateGrid() { return state_grid; }
  const emp::StateGrid & GetStateGrid() const { return state_grid; }
  double GetScore() const { return score; }
  emp::BitVector GetVisited() const { return sg_status.GetVisited(state_grid); }

  void SetPosition(size_t x, size_t y) { sg_status.SetPos(x,y); }
  void SetFacing(size_t facing) { sg_status.SetFacing(facing); }
  void SetStateGrid(const emp::StateGrid & in_sg) { state_grid = in_sg; }

  double GetFitness() {  // Setup the fitness function.
    ResetHardware();
    Process(2000);
    return score;
  }

  void ResetHardware() {
    base_t::ResetHardware();
    score = 0;
  }

  static void Inst_Move(SGOrg & org, const base_t::Instruction & inst) {
    emp_assert(org.state_grid.GetSize() > 0, org.state_grid.GetWidth(), org.state_grid.GetHeight());
    org.sg_status.Move(org.state_grid, 1);
    //org.sg_status.Move(org.state_grid, org.regs[inst.args[0]]);
  }

  static void Inst_Rotate(SGOrg & org, const base_t::Instruction & inst) {
    org.sg_status.Rotate(org.regs[inst.args[0]]);
  }

  static void Inst_Scan(SGOrg & org, const base_t::Instruction & inst) {
    int val = org.sg_status.Scan(org.state_grid);
    org.regs[inst.args[0]] = val;
    switch (val) {
    case -1: org.score -= 0.5; break;                                            // Poison
    case 0: break;                                                               // Eaten food
    case 1: org.score += 1.0; org.sg_status.SetState(org.state_grid, 0); break;  // Food! (now eaten)
    case 2: break;                                                               // Empty border
    case 3: org.score += 1.0; org.sg_status.SetState(org.state_grid, 2); break;  // Border with food
    }
  }

};
