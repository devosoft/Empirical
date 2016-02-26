//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build Symbulation-based SYMBiotic simULATION.
//
//  A 1 executed in the host organism increments org_score by 1.
//  A 0 executed in the host organism allows the symbiont to execute a single instruction.
//
//  A 1 executed in the symbiont increases org_score by the number of consecutive ones thus far.
//  A 0 exectued in the symbiont increases symb_score by the number of consecutive zeroes thus far.


#ifndef SYMBULATION_H
#define SYMBULATION_H

#include "../tools/assert.h"
#include "../tools/BitVector.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"

#include "OrgSignals.h"

namespace emp {
namespace evo {
  
  class SymbulationOrg {
  private:
    using callback_t = OrgSignals_Basic;

    BitVector host;
    BitVector symbiont;
    
    int host_pos;    // What bit position to execute next in the host?
    int symb_pos;    // What bit position to execute next in the symbiont?
    
    int host_score;  // Score accumulated by host, toward replication
    int symb_score;  // Score accumulated by symbiont, toward horizontal transmission

    int streak_0;    // Number of consecutive zeros executed by symbiont.
    int streak_1;    // Number of consecutive ones executed by symbiont.
    
  public:
    SymbulationOrg(const BitVector & genome)
      : host(genome), host_pos(0), symb_pos(0)
      , host_score(0), symb_score(0), streak_0(0), streak_1(0)
    {
      emp_assert(host.GetSize() > 0);
    }
    SymbulationOrg(Random & random, int size, double p=0.5)
      : host(RandomBitVector(random, size, p)), host_pos(0), symb_pos(0)
      , host_score(0), symb_score(0), streak_0(0), streak_1(0)
    {
      emp_assert(host.GetSize() > 0);
    }
    SymbulationOrg() = delete;
    SymbulationOrg(const SymbulationOrg &) = default;
    ~SymbulationOrg() { ; }

    int GetHostScore() const { return host_score; }
    int GetSymbiontScore() const { return symb_score; }

    void SetHost(const BitVector & genome, bool clear_symbiont=true) {
      host = genome;
      emp_assert(host.GetSize() > 0);
      host_pos = host_score = 0;
      if (clear_symbiont) {
	symbiont.Resize(0);
	symb_pos = symb_score = streak_0 = streak_1 = 0;
      }
    }

    void SetSymbiont(const BitVector & in_symb) {
      symbiont = in_symb;
      symb_pos = 0;
      symb_score = 0;
      streak_0 = streak_1 = 0;
    }
    
    // Try to inject a symbiont, but it might fail if another symbiont is already there.
    bool InjectSymbiont(const BitVector & in_symb, Random & random, double displace_prob=0.5) {
      // For a symbiont to be injectected successfully, there either has to be no symbiont
      // in the current cell -or- the existing symbiont must be displaced.
      if (symbiont.GetSize() == 0 || random.P(displace_prob)) {
	SetSymbiont(in_symb);
	return true;
      }
      return false;
    }
    
    void Execute(bool use_streaks=true, bool align_symbiont=false,
		 int host_self_bonus=1, int symb_self_bonus=1, int symb_host_bonus=1)
    {
      if (host[host_pos]) {                            // Host generating score for itself.
	host_score += host_self_bonus;
      }
      else if (symbiont.GetSize()) {                   // Host allowing extant symbiont to execute.
	// If a symbiont should exectue at the same position as a host, readjust.
	if (align_symbiont) {
	  symb_pos = host_pos % symbiont.GetSize();
	}

	// Determine next step based on symbiont bit
	if (symbiont[symb_pos]) {                      // Symbiont helping host.
	  if (use_streaks) {
	    streak_1++; streak_0 = 0;
	    host_score += streak_1 * symb_host_bonus;
	  }
	  else host_score += symb_host_bonus;
	}
	else {                                         // Symbiont helping itself.
	  if (use_streaks) {
	    streak_0++; streak_1 = 0;
	    symb_score += streak_0 * symb_self_bonus;
	  }
	  else symb_score += symb_self_bonus;
	}
	if (++symb_pos >= symbiont.GetSize()) symb_pos = 0;  // Advance symbiont position.
	
      }
      if (++host_pos >= host.GetSize()) host_pos = 0;  // Advance host position.

    }

  };
  
}
}


#endif
