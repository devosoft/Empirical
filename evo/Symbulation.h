//  This file is part of Empirical, https://github.com/devosoft/Empirical
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
//
//
//  Developer notes:
//  * Implement chance of vertical transmission.
//  * Implement chance of existing symbiont repelling horizontal transmission.
//  * Implement stats collection.

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
  public:
    using callback_t = OrgSignals_Eco;
  private:
    // Fixed members
    callback_t * callbacks; // Callbacks to population
    size_t id;            // Organism ID

    BitVector host;         // Current host genome
    BitVector symbiont;     // Current symbiont genome

    uint32_t host_cost;     // Score needed for host to replicate.
    uint32_t symb_cost;     // Score needed for symbiont to replicate.

    // Active members
    uint32_t host_pos;      // What bit position to execute next in the host?
    uint32_t symb_pos;      // What bit position to execute next in the symbiont?

    uint32_t host_score;    // Current host score, toward replication
    uint32_t symb_score;    // Current symbiont score, toward horizontal transmission

    uint32_t streak_00;     // Number of consecutive zeros executed by symbiont.
    uint32_t streak_01;     // Number of consecutive zeros executed by symbiont.
    uint32_t streak_1;      // Number of consecutive ones executed by symbiont.

  public:
    SymbulationOrg(const BitVector & genome, uint32_t h_cost, uint32_t s_cost=0)
      : callbacks(nullptr), id(0), host(genome), host_cost(h_cost), symb_cost(s_cost)
      , host_pos(0), symb_pos(0), host_score(0), symb_score(0)
      , streak_00(0), streak_01(0), streak_1(0)
    {
      emp_assert(genome.GetSize() > 0);
    }
    SymbulationOrg(const BitVector & genome) : SymbulationOrg(genome, (uint32_t)genome.size()) { ; }
    SymbulationOrg(Random & random, size_t size, double p, uint32_t h_cost, uint32_t s_cost)
      : SymbulationOrg(RandomBitVector(random, size, p), h_cost, s_cost) { ; }
    SymbulationOrg(Random & random, size_t size, double p=0.5)
      : SymbulationOrg(RandomBitVector(random, size, p), (uint32_t)size, 0) { ; }
    SymbulationOrg() = delete;
    SymbulationOrg(const SymbulationOrg &) = default;
    ~SymbulationOrg() { ; }

    void Setup(callback_t * in_callbacks, size_t in_id) {
      callbacks = in_callbacks;
      id = in_id;
    }

    void Reset(){
      host_pos = symb_pos = 0;
      host_score = symb_score = 0;
      streak_00 = streak_01 = streak_1 = 0;
    }

    const BitVector & GetHost() const { return host; }
    const BitVector & GetSymbiont() const { return symbiont; }

    uint32_t GetHostCost() const { return host_cost; }
    uint32_t GetSymbiontCost() const { return symb_cost; }
    uint32_t GetHostScore() const { return host_score; }
    uint32_t GetSymbiontScore() const { return symb_score; }

    void SetHost(const BitVector & genome, bool clear_symbiont=true) {
      host = genome;
      emp_assert(host.GetSize() > 0);
      host_pos = host_score = 0;
      if (clear_symbiont) {
        symbiont.Resize(0);
        symb_pos = symb_score = streak_00 = streak_01 = streak_1 = 0;
      }
    }

    void SetSymbiont(const BitVector & in_symb) {
      symbiont = in_symb;
      symb_pos = symb_score = streak_00 = streak_01 = streak_1 = 0;
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

    void TestHostRepro() {
      emp_assert(host_cost > 0);
      // Trigger reproduction if score is high enough.
      if (host_score >= host_cost) {
        Reset();                          // Reset before replication.
        callbacks->repro_sig.Trigger(id); // Trigger replication call.
      }
    }

    void TestSymbiontRepro() {
      emp_assert(symb_cost > 0);
      // Trigger reproduction if score is high enough.
      if (symb_score >= symb_cost) {
        symb_pos = symb_score = 0;                 // Reset Symbiont stats only.
        streak_00 = streak_01 = streak_1 = 0;
        callbacks->symbiont_repro_sig.Trigger(id); // Trigger symbiont replication call.
      }
    }

    void Execute(bool align_symbiont=false,
                 const std::function<uint32_t(uint32_t)> & symb_bonus00=[](uint32_t streak){ return streak; },
                 const std::function<uint32_t(uint32_t)> & host_bonus01=[](uint32_t streak){ return streak; },
                 const std::function<uint32_t(uint32_t)> & host_bonus1=[](uint32_t streak){ return 1; },
                 const std::function<uint32_t(uint32_t)> & symb_bonus01=[](uint32_t streak){ return 0; },
                 const std::function<uint32_t(uint32_t)> & host_bonus00=[](uint32_t streak){ return 0; }
               )
    {
      emp_assert(callbacks != nullptr);

      if (host[host_pos]) {                            // Host generating score for itself.
        streak_1++;
        host_score += host_bonus1(1);
        TestHostRepro();
      }
      else {
        streak_1 = 0;
        if (symbiont.GetSize()) {                   // Host allowing extant symbiont to execute.
          // If a symbiont should exectue at the same position as a host, readjust.
	        if (align_symbiont) symb_pos = host_pos % symbiont.GetSize();

	        // Determine next step based on symbiont bit
	        if (symbiont[symb_pos]) {                      // Symbiont helping host.
            streak_01++; streak_00 = 0;
            host_score += host_bonus01(streak_01);
            symb_score += symb_bonus01(streak_01);
	        }
	        else {                                         // Symbiont helping itself.
            streak_00++; streak_01 = 0;
            host_score += host_bonus00(streak_00);
            symb_score += symb_bonus00(streak_00);
	        }
          TestHostRepro();
          TestSymbiontRepro();
	        if (++symb_pos >= symbiont.GetSize()) symb_pos = 0;  // Advance symbiont position.
        }
      }
      // else std::cout << "No Symbiont";
      if (++host_pos >= host.GetSize()) host_pos = 0;  // Advance host position.

      // std::cout << "host_score=" << host_score
      //           << "  symb_score=" << symb_score
      //           << std::endl;
    }

    void Print(std::ostream & os) const {
      os << "Host: " << host << std::endl
         << "Symbiont: " << symbiont << std::endl;
    }

  };

  std::ostream & operator<<(std::ostream & os, const SymbulationOrg & org) {
    org.Print(os);
    return os;
  }

}
}


#endif
