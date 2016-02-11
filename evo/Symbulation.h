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

namespace emp {
namespace evo {
  
  class SymbulationOrg {
  private:
    BitVector org;
    BitVector symbiont;
    
    int org_pos;     // What bit position to execute next in the host?
    int symb_pos;    // What bit position to execute next in the symbiont?
    
    int org_score;   // Score accumulated by host, toward replication
    int symb_score;  // Score accumulated by symbiont, toward horizontal transmission

    int streak_0;    // Number of consecutive zeros executed by symbiont.
    int streak_1;    // Number of consecutive ones executed by symbiont.
    
  public:
    SymbulationOrg() : org_pos(0), symb_pos(0), org_score(0), symb_score(0),
		       streak_0(0), streak_1(0) { ; }
    ~SymbulationOrg() { ; }

    int GetOrgScore() const { return org_score; }
    int GetSymbiontScore() const { return symb_score; }

    Execute() {
      if (org[org_pos]) {  // Host generating score for itself.
	org_score++;
      }
      else {               // Host allowing symbiont to execute.
	if (!symbiont.GetSize()) break;  // No symbiont in this host.
	
	if (symbiont[symb_pos]) {   // Symbiont helping host.
	  streak_1++;
	  streak_0 = 0;
	  org_score += streak_1;
	}
	else {                      // Symbiont helping itself.
	  streak_0++;
	  streak_1 = 0;
	  symb_score += streak_0;
	}
	if (++symb_pos >= symbiont.GetSize()) symb_pos = 0;  // Advance symbiont position.
	
      }
      if (++org_pos >= org.GetSize()) org_pos = 0;  // Advance org position.
    }
  };
  
}
}


#endif
