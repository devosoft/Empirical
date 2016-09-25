//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population managers for use with emp::evo::World
//
//
//  Developer notes:
//  * Rather than deleting organisms outright, run all deletions through a ClearCell function
//    so that a common signal system can also be run.
//  * Grids always add injected organisms to empty cells; may have trouble if there are none.
//  * Population iterators and indexing should either be const OR work with proxies to ensure
//    that fitness caching and signals are handled correctly.
//  * Should we do something to link Proxy and iterator; in both cases they represent a position
//    in the populaiton vector, but iterator can only point to valid cells...

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

#include "../tools/random_utils.h"
#include "../tools/Signal.h"

#include "PopulationIterator.h"

namespace emp {
namespace evo {

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    using pop_t = emp::vector<ptr_t>;

    pop_t pop;
    FIT_MANAGER & fitM;

    Random * random_ptr;

    class Proxy {
    private:
      PopulationManager_Base<ORG,FIT_MANAGER> & popM;  // Which pop manager is this proxy from?
      int id;                                          // Which position does it represent?
    public:
      Proxy(PopulationManager_Base & _p, int _id) : popM(_p), id(_id) { ; }
      operator ORG*() const { return popM.pop[id]; }
      ORG * operator->() const { return popM.pop[id]; }
      ORG & operator*() const { return *(popM.pop[id]); }
      Proxy & operator=(ORG * new_org) { popM.AddOrgAt(new_org,id); return *this; }
    };

  public:
    PopulationManager_Base(const std::string &, FIT_MANAGER & _fm) : fitM(_fm) { ; }
    ~PopulationManager_Base() { Clear(); }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ptr_t;

    using iterator_t = PopulationIterator<PopulationManager_Base<ORG,FIT_MANAGER> >;
    friend class interator_t;

    int GetSize() const { return (int) pop.size(); }

    void SetRandom(Random * r) { random_ptr = r; }
    void Setup(Random * r) { SetRandom(r); }

    // AddOrgAt and AddOrgAppend are the only ways new organisms come into a population (all others
    // go through these)

    int AddOrgAt(ORG * new_org, int pos) {
      emp_assert(pos < (int) pop.size());   // Make sure we are placing into a legal position.
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      fitM.Clear(pos);
      return pos;
    }

    int AddOrgAppend(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      fitM.Clear(pos);
      return pos;
    }

    // In practice, we should always use AddOrg and AddOrgBirth which are setup appropriately
    // for each population type.
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.

    int AddOrg(ORG * new_org) { return AddOrgAppend(new_org); }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = random_ptr->GetInt((int) pop.size());
      return AddOrgAt(new_org, pos);
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * org : pop) if (org) delete org;  // Delete current organisms.
      pop.resize(0);                              // Remove deleted organisms.
      fitM.Clear();                               // Clear the fitness manager cache.
    }
    void Resize(size_t new_size) {
      emp_assert(new_size >= 0);
      const auto old_size = pop.size();
      for (int i = new_size; i < old_size; i++) {
        delete pop[i];                            // Delete organisms being removed.
        fitM.Clear(i);                            // Clear fitness cache for Deleted cells.
      }
      pop.resize(new_size, nullptr);  // Initialize new orgs as null.
    }

    void Update() { ; } // Basic version of Update() does nothing, but World may trigger actions.

    // Execute() redirect to all organisms in the population, forwarding arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (ORG * org : pop) {
        if (org) org->Execute(std::forward<ARGS>(args)...);
      }
    }

    // --- POPULATION ANALYSIS ---

    emp::vector<int> FindCellIDs(const std::function<bool(ORG*)> & filter) {
      emp::vector<int> valid_IDs(0);
      for (size_t i = 0; i < pop.size(); i++) {
        if (filter(pop[i])) valid_IDs.push_back(i);
      }
      return valid_IDs;
    }
    emp::vector<int> GetValidOrgIDs() { return FindCellIDs([](ORG*org){ return org != nullptr; }); }
    emp::vector<int> GetEmptyPopIDs() { return FindCellIDs([](ORG*org){ return org == nullptr; }); }

    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const int new_size, bool choose_random=true) {
      if (new_size >= (int) pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Delete all of the organisms we are removing and resize the population.
      for (int i = new_size; i < (int) pop.size(); ++i) { delete pop[i]; }
      pop.resize(new_size);

      fitM.Clear();  // Everyone is either deleted or in the wrong place!
    }

    // --- PRINTING ---

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream & os = std::cout,
              std::string empty="X", std::string spacer=" ") {
      for (ORG * org : pop) {
        if (org) os << string_fun(org);
        else os << empty;
        os << spacer;
      }
    }
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      Print( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
    }
    void PrintOrgCounts(std::function<std::string(ORG*)> string_fun, std::ostream & os = std::cout) {
      std::map<ORG,int> org_counts;
      for (ORG * org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
      for (ORG * org : pop) if (org) org_counts[*org] += 1; // Count actual types.
      for (auto x : org_counts) {
        ORG cur_org = x.first;
        os << string_fun(&cur_org) << " : " << x.second << std::endl;
      }
    }

    // --- FOR VECTOR COMPATIBILITY ---
    size_t size() const { return pop.size(); }
    void resize(int new_size) { Resize(new_size); }
    void clear() { Clear(); }

    // @CAO: these need work to make sure we send correct signals on changes & update fitness cache.
    Proxy operator[](int i) { return Proxy(*this, i); }
    const ptr_t operator[](int i) const { return pop[i]; }
    iterator_t begin() { return iterator_t(this, 0); }
    iterator_t end() { return iterator_t(this, pop.size()); }
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
    PopulationManager_Plugin(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm)
    , sig_clear(to_string(_w_name, "::pop_clear"))
    , sig_update(to_string(_w_name, "::pop_update"))
    , sig_add_org(to_string(_w_name, "::pop_add_org"))
    , sig_add_org_birth(to_string(_w_name, "::pop_add_org_birth"))
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
    PopulationManager_EA(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm) { ; }
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
      fitM.Clear();
    }

    void Update() {
      for (ORG * m : pop) delete m;  // Delete the current population.
      pop = next_pop;                // Move over the next generation.
      next_pop.resize(0);            // Clear out the next pop to refill again.
      fitM.Clear();                  // Clear the fitness given new cells.
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

    int max_size;
    int bottleneck_size;
    int num_bottlenecks;
  public:
    PopulationManager_SerialTransfer(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm)
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
        base_t::DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      return AddOrgAppend(new_org);
    }
  };

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Grid : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;
    using base_t::random_ptr;

    int width;
    int height;

  public:
    PopulationManager_Grid(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm) {
      ConfigPop(10,10);
    }
    ~PopulationManager_Grid() { ; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void ConfigPop(int w, int h) { width = w; height = h; pop.resize(width*height, nullptr); }

    // Injected orgs go into a random position.
    int AddOrg(ORG * new_org) {
      emp::vector<int> empty_spots = base_t::GetEmptyPopIDs();
      const int pos = (empty_spots.size()) ?
        empty_spots[ random_ptr->GetUInt(empty_spots.size()) ] : random_ptr->GetUInt(pop.size());
      return base_t::AddOrgAt(new_org, pos);
    }

    // Newly born orgs go next to their parents.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int offset = random_ptr->GetInt(9);
      const int offspring_x = emp::mod(parent_pos%width + offset%3 - 1, width);
      const int offspring_y = emp::mod(parent_pos/width + offset/3 - 1, height);
      const int pos = offspring_x + offspring_y*width;

      return base_t::AddOrgAt(new_org, pos);
    }

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream& os=std::cout,
               const std::string & empty="-", const std::string & spacer=" ") {
      emp_assert(string_fun);
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
          ORG * org = pop[x+y*width];
          if (org) os << string_fun(org) << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }

    void Print(std::ostream& os=std::cout, const std::string & empty="X", std::string spacer=" ") {
      Print( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
    }
  };

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Pools : public PopulationManager_Base<ORG,FIT_MANAGER> {
  public:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::pop;
    using base_t::fitM;
    using base_t::random_ptr;

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
    PopulationManager_Pools(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm), org_count(0) { ; }
    ~PopulationManager_Pools() { ; }

    int GetPoolCount() const { return pool_count; }
    const vector<int> & GetSizes() const { return pool_sizes ; }
    int GetUpper() const { return r_upper; }
    int GetLower() const { return r_lower; }

    void Setup(Random * r) {
      base_t::SetRandom(r);
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

      org_count++;
      return AddOrgAt(new_org, pos);
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
      return AddOrgAt(new_org, pos);
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
