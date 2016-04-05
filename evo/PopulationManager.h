//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population manages for use with emp::evo::World

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

#include "../tools/random_utils.h"

namespace emp {
namespace evo {

  template <typename ORG>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    emp::vector<ORG *> pop;

    Random * random_ptr;

  public:
    PopulationManager_Base() { ; }
    ~PopulationManager_Base() { ; }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }

    void SetRandom(Random * r) { random_ptr = r; }

    // AddOrg and ReplaceOrg should be the only ways new organisms come into a population.
    // AddOrg inserts them into the end of the designated population.
    // ReplaceOrg places them at a specific position, replacing anyone who may already be there.
    int AddOrg(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
    int AddOrgBirth(ORG * new_org) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * m : pop) delete m;
      pop.resize(0);
    }

    void Update() { ; } // Basic version of Update() does nothing, but World may trigger actions.

    // Execute() redirect to all organisms in the population, forwarding arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (ORG * m : pop) {
        m->Execute(std::forward<ARGS>(args)...);
      }
    }


    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottlenect to (potentiall) shrink it.
    void DoBottleneck(const int new_size, bool choose_random=true) {
      if (choose_random) {
        std::cout << "Ping 1! " << new_size << std::endl;
        emp::Shrink<ptr_t>(*random_ptr, pop, new_size);
      } else {
        if (new_size < (int) pop.size()) pop.resize(new_size);
      }
    }
  };

  // A standard population manager for using synchronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename ORG>
  class PopulationManager_EA : public PopulationManager_Base<ORG> {
  protected:
    emp::vector<ORG *> next_pop;
    using PopulationManager_Base<ORG>::pop;

  public:
    PopulationManager_EA() { ; }
    ~ PopulationManager_EA() { Clear(); }

    int AddOrgBirth(ORG * new_org) {
      const int pos = next_pop.size();
      next_pop.push_back(new_org);
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * m : pop) delete m;
      for (ORG * m : next_pop) delete m;

      pop.resize(0);
      next_pop.resize(0);
    }

    void Update() {
      for (ORG * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
    }
  };


  // A standard population manager for using a serial-transfer protocol.  All new
  // organisms get inserted into the main population; once it is full the population
  // is shrunk down.

  template <typename ORG>
  class PopulationManager_SerialTransfer : public PopulationManager_Base<ORG> {
  protected:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::DoBottleneck;

    int max_size;
    int bottleneck_size;
    int num_bottlenecks;
  public:
    PopulationManager_SerialTransfer()
      : max_size(1000), bottleneck_size(100), num_bottlenecks(0) { ; }
    ~PopulationManager_SerialTransfer() { ; }

    int GetMaxSize() const { return max_size; }
    int GetBottleneckSize() const { return bottleneck_size; }
    int GetNumBottlnecks() const { return num_bottlenecks; }

    void SetMaxSize(const int m) { max_size = m; }
    void SetBottleneckSize(const int b) { bottleneck_size = b; }

    void Config(int m, int b) { max_size = m; bottleneck_size = b; }

    int AddOrgBirth(ORG * new_org) {
      if (pop.size() >= max_size) {
        DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
  };

}
}


#endif
