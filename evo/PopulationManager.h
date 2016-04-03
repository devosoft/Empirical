//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population manages for use with emp::evo::World

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

namespace emp {
namespace evo {

  template <typename MEMBER>
  class PopulationManager_Base {
  public:
    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;

    using ptr_t = MEMBER *;
    emp::vector<MEMBER *> pop;

    Random * random_ptr;

  public:
    PopulationManager_Base() { ; }
    ~PopulationManager_Base() { ; }

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }

    void SetRandom(Random * r) { random_ptr = r; }

    // AddOrg and ReplaceOrg should be the only ways new organisms come into a population.
    // AddOrg inserts them into the end of the designated population.
    // ReplaceOrg places them at a specific position, replacing anyone who may already be there.
    int AddOrg(MEMBER * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
    int AddOrgBirth(MEMBER * new_org) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (MEMBER * m : pop) delete m;
      pop.resize(0);
    }

    void Update() { ; } // Basic version of Update() does nothing, but World may trigger actions.

    // Execute() redirect to all organisms in the population, forwarding arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (MEMBER * m : pop) {
        m->Execute(std::forward<ARGS>(args)...);
      }
    }
  };

  // A standard population manager for using sychronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename MEMBER>
  class PopulationManager_EA : public PopulationManager_Base<MEMBER> {
  public:
    emp::vector<MEMBER *> next_pop;
    using PopulationManager_Base<MEMBER>::pop;

  public:
    PopulationManager_EA() { ; }
    ~ PopulationManager_EA() { Clear(); }

    int AddOrgBirth(MEMBER * new_org) {
      const int pos = next_pop.size();
      next_pop.push_back(new_org);
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (MEMBER * m : pop) delete m;
      for (MEMBER * m : next_pop) delete m;

      pop.resize(0);
      next_pop.resize(0);
    }

    void Update() {
      for (MEMBER * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
    }
  };

}
}


#endif
