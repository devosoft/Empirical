//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population managers for use with emp::evo::World
//
//
//  Developer notes:
//  * Rather than deleting organisms ourright, run all deletions through a ClearCell function
//    so that a common signal system can also be run.

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

#include "../tools/random_utils.h"
#include "PopulationIterator.h"

namespace emp {
namespace evo {

  template <typename POP_MANAGER> class PopulationIterator;

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    using pop_t = emp::vector<ORG *>;

    pop_t pop;
    FIT_MANAGER fitM;

    Random * random_ptr;

  public:
    PopulationManager_Base(const std::string & world_name) { (void) world_name; }
    ~PopulationManager_Base() { Clear(); }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ORG*;

    friend class PopulationIterator< PopulationManager_Base<ORG,FIT_MANAGER> >;
    using iterator = PopulationIterator<PopulationManager_Base<ORG,FIT_MANAGER> >;

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, pop.size()); }

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }
    void clear() { pop.clear(); }
    int GetSize() const { return (int) pop.size(); }

    void SetRandom(Random * r) { random_ptr = r; }
    void Setup(Random * r) { SetRandom(r); }

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream & os = std::cout,
              std::string empty="X", std::string spacer=" ") {
      for (ORG * org : pop) {
        if (org) os << string_fun(org);
        else os << empty;
        os << spacer;
      }
    }
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      for (ORG * org : pop) {
        if (org) os << *org;
        else os << empty;
        os << spacer;
      }
    }

    // AddOrg and AddOrgBirth should be the only ways new organisms come into a population.
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.
    int AddOrg(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      fitM.ClearCache(pos);
      return pos;
    }
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      fitM.ClearCache(pos);
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * org : pop) if (org) delete org;
      pop.resize(0);
      fitM.ClearCache();
    }

    void Update() { ; } // Basic version of Update() does nothing, but World may trigger actions.

    // Execute() redirect to all organisms in the population, forwarding arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (ORG * org : pop) {
        if (org) org->Execute(std::forward<ARGS>(args)...);
      }
    }

    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const int new_size, bool choose_random=true) {
      if (new_size >= (int) pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Delete all of the organisms we are removing and resize the population.
      for (int i = new_size; i < (int) pop.size(); ++i) { delete pop[i]; }
      pop.resize(new_size);

      fitM.ClearCache();  // Everyone is either deleted or in the wrong place!
    }
  };


  // A population manager that is defined elsewhere, for use with plugins.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Plugin : public PopulationManager_Base<ORG, FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;

    // Most of the key functions in the population manager can be interfaced with symbols.  If you
    // need to modify the more complex behaviors (such as Execute) you need to create a new
    // derrived class from PopulationManager_Base, which is also legal in a plugin.
    Signal<emp::vector<ORG*>&> sig_clear;
    Signal<emp::vector<ORG*>&> sig_update;
    Signal<emp::vector<ORG*>&, ORG*, int&> sig_add_org;            // args: new org, return: offspring pos
    Signal<emp::vector<ORG*>&, ORG*, int, int&> sig_add_org_birth; // args: new org, parent pos, return: offspring pos

  public:
    PopulationManager_Plugin(const std::string & world_name)
    : base_t(world_name)
    , sig_clear(to_string(world_name, "::pop_clear"))
    , sig_update(to_string(world_name, "::pop_update"))
    , sig_add_org(to_string(world_name, "::pop_add_org"))
    , sig_add_org_birth(to_string(world_name, "::pop_add_org_birth"))
    { ; }
    ~PopulationManager_Plugin() { Clear(); }

    LinkKey OnClear(const std::function<void(emp::vector<ORG*>&)> & fun) {
      return sig_clear.AddAction(fun);
    }
    LinkKey OnUpdate(const std::function<void(emp::vector<ORG*>&)> & fun) {
      return sig_update.AddAction(fun);
    }
    LinkKey OnAddOrg(const std::function<void(emp::vector<ORG*>&, ORG*, int&)> & fun) {
      return sig_add_org.AddAction(fun);
    }
    LinkKey OnAddOrgBirth(const std::function<void(emp::vector<ORG*>&, ORG*, int, int&)> & fun) {
      return sig_add_org_birth.AddAction(fun);
    }

    void Clear() {
      if (sig_clear.GetNumActions()) sig_clear.Trigger(pop);
      else base_t::Clear();  // If no actions are linked to sig_clear, use default.
    }
    void Update() { sig_update.Trigger(pop); }

    int AddOrg(ORG * new_org) {
      int new_pos;
      sig_add_org.Trigger(pop, new_org, new_pos);
      return new_pos;
    }
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      int offspring_pos;
      sig_add_org_birth.Trigger(pop, new_org, parent_pos, offspring_pos);
      return offspring_pos;
    }
  };

  // A standard population manager for using synchronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_EA : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;

    emp::vector<ORG *> next_pop;

  public:
    PopulationManager_EA(const std::string & world_name) : base_t(world_name) { ; }
    ~PopulationManager_EA() { Clear(); }

    static constexpr bool emp_has_separate_generations = true;

    int AddOrgBirth(ORG * new_org, int parent_pos) {
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
      fitM.ClearCache();
    }

    void Update() {
      for (ORG * m : pop) delete m;  // Delete the current population.
      pop = next_pop;                // Move over the next generation.
      next_pop.resize(0);            // Clear out the next pop to refill again.
      fitM.ClearCache();             // Clear the fitness cache.
    }
  };


  // A standard population manager for using a serial-transfer protocol.  All new
  // organisms get inserted into the main population; once it is full the population
  // is shrunk down.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_SerialTransfer : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;
    using base_t::random_ptr;
    using base_t::DoBottleneck;
    using base_t::SetRandom;

    int max_size;
    int bottleneck_size;
    int num_bottlenecks;
  public:
    PopulationManager_SerialTransfer(const std::string & world_name)
    : base_t(world_name)
    , max_size(1000), bottleneck_size(100), num_bottlenecks(0) { ; }
    ~PopulationManager_SerialTransfer() { ; }

    int GetMaxSize() const { return max_size; }
    int GetBottleneckSize() const { return bottleneck_size; }
    int GetNumBottlnecks() const { return num_bottlenecks; }

    void SetMaxSize(const int m) { max_size = m; }
    void SetBottleneckSize(const int b) { bottleneck_size = b; }

    void ConfigPop(int m, int b) { max_size = m; bottleneck_size = b; }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      if ((int) pop.size() >= max_size) {
        DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
  };

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Grid : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;
    using base_t::random_ptr;
    using base_t::SetRandom;

    int width;
    int height;

    int ToX(int id) const { return id % width; }
    int ToY(int id) const { return id / width; }
    int ToID(int x, int y) const { return y*width + x; }

  public:
    PopulationManager_Grid(const std::string & world_name) : base_t(world_name) {
      ConfigPop(10,10);
    }
    ~PopulationManager_Grid() { ; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void ConfigPop(int w, int h) { width = w; height = h; pop.resize(width*height, nullptr); }

    // Injected orgs go into a random position.
    int AddOrg(ORG * new_org) {
      emp::vector<int> empty_spots = GetValidOrgIndices();
      const int pos = empty_spots[ random_ptr->GetInt((int) empty_spots.size()) ];

      pop[pos] = new_org;
      fitM.ClearCache(pos);
      return pos;
    }

    // Newly born orgs go next to their parents.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int parent_x = ToX(parent_pos);
      const int parent_y = ToY(parent_pos);
      const int offset = random_ptr->GetInt(9);
      const int offspring_x = emp::mod(parent_x + offset%3 - 1, width);
      const int offspring_y = emp::mod(parent_y + offset/3 - 1, height);
      const int pos = ToID(offspring_x, offspring_y);

      if (pop[pos]) delete pop[pos];

      pop[pos] = new_org;
      fitM.ClearCache(pos);

      return pos;
    }

    emp::vector<int> GetValidOrgIndices(){
      emp::vector<int> valid_orgs(0);
      for (int i = 0; i < pop.size(); i++){
        if (pop[i] == nullptr){
          valid_orgs.push_back(i);
        }
      }
      return valid_orgs;
    }

    void Print(std::function<std::string(ORG*)> string_fun,
               std::ostream & os = std::cout,
               const std::string & empty="-",
               const std::string & spacer=" ")
    {
      emp_assert(string_fun);
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
          ORG * org = pop[ToID(x,y)];
          if (org) os << string_fun(org) << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }

    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
          ORG * org = pop[ToID(x,y)];
          if (org) os << *org << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }
  };

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Pools : public PopulationManager_Base<ORG,FIT_MANAGER> {
  public:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;
    using base_t::random_ptr;
    using base_t::SetRandom;
    using base_t::GetSize;

    int pool_count;                            // How many pools are in the population?
    vector<int> pool_sizes;                    // How large is each pool?
    std::map<int, vector<int> > connections;   // Which other pools can each position access?
    int org_count;                             // How many organisms have beeen inserted into population?
    int r_upper;                               // How large can a random pool size be?
    int r_lower;                               // How small can a random pool size be?
    vector<int> pool_end;                      // Where does the next pool begin? First begins at 0.
    double mig_rate;                           // How often do organisms migrate to a connected pool?
    vector<int> pool_id;

  public:
    PopulationManager_Pools(const std::string & world_name) : base_t(world_name), org_count(0) { ; }
    ~PopulationManager_Pools() { ; }

    int GetPoolCount() const { return pool_count; }
    const vector<int> & GetSizes() const { return pool_sizes ; }
    int GetUpper() const { return r_upper; }
    int GetLower() const { return r_lower; }

    void Setup(Random * r) {
        SetRandom(r);
        vector<int>* temp_sizes = new vector<int>;
        std::map<int, vector<int> > temp_connect;

      ConfigPop(5, *temp_sizes, &temp_connect, 150, 10, 0.05, 200);
    }

    // Sets up population based on user specs.
    void ConfigPop(int pc, vector<int> ps, std::map<int, vector<int> > * c, int u, int l,
                   double mg, int pop_size) {
      pool_count = pc;
      pool_sizes = ps;
      r_upper = u;
      r_lower = l;
      connections = *c;
      mig_rate = mg;
      pool_end = {};

      vector<int> temp (pop_size, 0);
      pool_id = temp;

      pop.resize(pop_size, nullptr);

      // If no pool sizes in vector, defaults to random sizes for each
      if (pool_sizes.size() == 0) {
        while (true) {
          int pool_total = 0;
          for( int i = 0; i < pool_count - 1; i++){
            pool_sizes.push_back(40);
            pool_total += pool_sizes[i];
          }

          if (pool_total < pop_size){ //Keep generating random sizes until true
            pool_sizes.push_back(pop_size - pool_total);
            break;
          }

          for (int i = 0; i < pool_count - 1; i++) { pool_sizes.pop_back(); }
        }
      }
      // If only one pool size in vector, uses that size for all pools
      else if (pool_sizes.size() == 1) {
        int temp = pool_sizes[0];
        for (int i = 1; i < pool_count; i++) { pool_sizes.push_back(temp); }
      }
      else if (pool_sizes.size() != pool_count) {
        std::cerr << " ERROR: Not enough pool sizes" << std::endl;
        return;
      }

      int total = 0;
      for (auto el : pool_sizes) { total += el; }

      emp_assert(pop_size == total && "POP_SIZE is different than total pool sizes");

      // Divide World into pools
      int arr_size = 0;
      int prev_size = 0;
      int pool_num = 0;
      for (auto el : pool_sizes) {
        arr_size += el;
        for( int i = prev_size; i < arr_size; i++){
            pool_id[i] = pool_num;
        }
        prev_size = arr_size;
        pool_num++;
        pool_end.push_back(arr_size);
      }
      return;
    }

    // Injected orgs go into a random pool.
    int AddOrg(ORG * new_org) {
      int range_u;
      int range_l = 0;

      // Ensure that each pool has at least one organism before adding to old pools.
      if (org_count < pool_count) {
        range_u = pool_end[org_count];
        if (org_count > 0) { range_l = pool_end[org_count-1]; }
        // @CAO: Shouldn't we just insert the organism in the new pool and return?
      }
      else {
        range_u = (int) pop.size();
      }

      const int pos = random_ptr->GetInt(range_l, range_u);

      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      org_count++;
      fitM.ClearCache(pos);
      return pos;
    }

    // Newly born orgs have a chance to migrate to a connected pool.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      int InsertPool = 0;  // Which pool should new org be born into?

      // Test if a migration should happen ; if so, determine new pool.
      const auto & parent_conns = connections[parent_pos];
      if (random_ptr->P(mig_rate) && parent_conns.size() > 0) {
        int conn_id = random_ptr->GetInt(0, parent_conns.size());
        InsertPool = parent_conns[conn_id];
        }
        else{ InsertPool = pool_id[parent_pos]; }

      int range_l = InsertPool ? pool_end[InsertPool-1] : 0;
      int range_u = pool_end[InsertPool];

      const int pos = random_ptr->GetInt(range_l, range_u);
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;

      fitM.ClearCache(pos);
      return pos;
    }
 };

  using PopBasic  = PopulationManager_Base<int>;
  using PopPlugin = PopulationManager_Plugin<int>;
  using PopEA     = PopulationManager_EA<int>;
  using PopST     = PopulationManager_SerialTransfer<int>;
  using PopGrid   = PopulationManager_Grid<int>;
  using PopPool   = PopulationManager_Pools<int>;

}
}


#endif
