//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population managers for use with emp::evo::World
//
//
//  Developer notes:
//  * Should we do something to link Proxy and iterator; in both cases they represent a position
//    in the populaiton vector, but iterator can only point to valid cells...

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

#include "../tools/random_utils.h"
#include "../tools/Range.h"
#include "../tools/signal.h"

#include "PopulationIterator.h"

namespace emp {
namespace evo {

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    using pop_t = emp::vector<ptr_t>;

    FIT_MANAGER & fitM;
    size_t num_orgs;
    Random * random_ptr;

  private:
    pop_t pop;

    class Proxy {
    private:
      PopulationManager_Base<ORG,FIT_MANAGER> & popM;  // Which pop manager is this proxy from?
      size_t id;                                       // Which position does it represent?
    public:
      Proxy(PopulationManager_Base & _p, size_t _id) : popM(_p), id(_id) { ; }
      operator ORG*() const { return popM.pop[id]; }
      ORG * operator->() const { return popM.pop[id]; }
      ORG & operator*() const { return *(popM.pop[id]); }
      Proxy & operator=(ORG * new_org) { popM.AddOrgAt(new_org,id); return *this; }
    };

  public:
    PopulationManager_Base(const std::string &, FIT_MANAGER & _fm) : fitM(_fm), num_orgs(0) { ; }
    ~PopulationManager_Base() { Clear(); }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ptr_t;

    using iterator_t = PopulationIterator<PopulationManager_Base<ORG,FIT_MANAGER> >;

    size_t GetSize() const { return pop.size(); }
    size_t GetNumOrgs() const { return num_orgs; }
    ptr_t GetOrg(size_t id) const { return pop[id]; }
    double CalcFitness(size_t id, const std::function<double(ORG*)> & fit_fun) const {
      return fitM.CalcFitness(id, pop[id], fit_fun);
    }

    void SetRandom(Random * r) { random_ptr = r; }
    void Setup(Random * r) { SetRandom(r); }

    // AddOrgAt, AddOrgAppend, and SetOrgs are the only ways new organisms come into a population
    // (all others go through these)

    size_t AddOrgAt(ORG * new_org, size_t pos) {
      emp_assert(pos < pop.size());   // Make sure we are placing into a legal position.
      if (pop[pos]) { delete pop[pos]; --num_orgs; }
      pop[pos] = new_org;
      fitM.ClearAt(pos);
      ++num_orgs;
      return pos;
    }

    size_t AddOrgAppend(ORG * new_org) {
      const size_t pos = pop.size();
      pop.push_back(new_org);
      fitM.ClearAt(pos);
      ++num_orgs;
      return pos;
    }

    void SetOrgs(const emp::vector<ORG*> & new_pop) {
      Clear();
      pop = new_pop;
      num_orgs = 0;
      for (ptr_t x : pop) if (x) num_orgs++;
    }

    // Likewise, Clear and ClearOrgAt are the only ways to remove organisms...
    void Clear() {
      // Delete all organisms.
      for (ORG * org : pop) if (org) delete org;  // Delete current organisms.
      pop.resize(0);                              // Remove deleted organisms.
      fitM.ClearPop();                               // Clear the fitness manager cache.
      num_orgs = 0;
    }

    void ClearOrgAt(size_t pos) {
      // Delete all organisms.
      if (pop[pos]) { delete pop[pos]; pop[pos]=nullptr; num_orgs--; }  // Delete current organisms.
      fitM.ClearAt(pos);
    }

    // In practice, we should always use AddOrg and AddOrgBirth which are setup appropriately
    // for each population type.
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.

    size_t AddOrg(ORG * new_org) { return AddOrgAppend(new_org); }

    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      const size_t pos = random_ptr->GetUInt(pop.size());
      return AddOrgAt(new_org, pos);
    }

    void Resize(size_t new_size) {
      emp_assert(new_size >= 0);
      for (size_t i = new_size; i < pop.size(); i++) ClearOrgAt(i); // Remove orgs out or range.
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

    // -- Random Access --

    // Get any cell, at random
    size_t GetRandomCell() const { return random_ptr->GetInt(0, pop.size()); }

    // By default, assume a well-mixed population so random neighbors can be anyone.
    size_t GetRandomNeighbor(size_t /*id*/) const { return random_ptr->GetInt(0, pop.size()); }

    // Get random *occupied* cell.
    size_t GetRandomOrg() const {
      emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
      size_t pos = random_ptr->GetUInt(0, pop.size());
      while (pop[pos] == nullptr) pos = random_ptr->GetUInt(0, pop.size());
      return pos;
    }

    // --- POPULATION ANALYSIS ---

    emp::vector<size_t> FindCellIDs(const std::function<bool(ORG*)> & filter) {
      emp::vector<size_t> valid_IDs(0);
      for (size_t i = 0; i < pop.size(); i++) {
        if (filter(pop[i])) valid_IDs.push_back(i);
      }
      return valid_IDs;
    }
    emp::vector<size_t> GetValidOrgIDs() { return FindCellIDs([](ORG*org){ return org != nullptr; }); }
    emp::vector<size_t> GetEmptyPopIDs() { return FindCellIDs([](ORG*org){ return org == nullptr; }); }

    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const size_t new_size, bool choose_random=true) {
      if (new_size >= pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Clear out all of the organisms we are removing and resize the population.
      for (size_t i = new_size; i < pop.size(); ++i) ClearOrgAt(i);
      pop.resize(new_size);

      fitM.ClearPop();  // Everyone is either removed or in the wrong place!
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
      std::map<ORG,size_t> org_counts;
      for (ORG * org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
      for (ORG * org : pop) if (org) org_counts[*org] += 1; // Count actual types.
      for (auto x : org_counts) {
        ORG cur_org = x.first;
        os << string_fun(&cur_org) << " : " << x.second << std::endl;
      }
    }

    // --- FOR VECTOR COMPATIBILITY ---
    size_t size() const { return pop.size(); }
    void resize(size_t new_size) { Resize(new_size); }
    void clear() { Clear(); }

    Proxy operator[](size_t i) { return Proxy(*this, i); }
    const ptr_t operator[](size_t i) const { return pop[i]; }
    iterator_t begin() { return iterator_t(this, 0); }
    iterator_t end() { return iterator_t(this, pop.size()); }
  };


  // A population manager that is defined elsewhere, for use with plugins.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Plugin : public PopulationManager_Base<ORG, FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;

    // Most of the key functions in the population manager can be interfaced with symbols.  If you
    // need to modify the more complex behaviors (such as Execute) you need to create a new
    // derrived class from PopulationManager_Base, which is also legal in a plugin.
    Signal<> sig_clear;
    Signal<> sig_update;
    Signal<ORG*, size_t&> sig_add_org;               // args: new org, return: offspring pos
    Signal<ORG*, size_t, size_t&> sig_add_org_birth; // args: new org, parent pos, return: offspring pos

  public:
    PopulationManager_Plugin(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm)
    , sig_clear(to_string(_w_name, "::pop_clear"))
    , sig_update(to_string(_w_name, "::pop_update"))
    , sig_add_org(to_string(_w_name, "::pop_add_org"))
    , sig_add_org_birth(to_string(_w_name, "::pop_add_org_birth"))
    { ; }
    ~PopulationManager_Plugin() { Clear(); }

    LinkKey OnClear(const std::function<void()> & fun) {
      return sig_clear.AddAction(fun);
    }
    LinkKey OnUpdate(const std::function<void()> & fun) {
      return sig_update.AddAction(fun);
    }
    LinkKey OnAddOrg(const std::function<void(ORG*, size_t&)> & fun) {
      return sig_add_org.AddAction(fun);
    }
    LinkKey OnAddOrgBirth(const std::function<void(ORG*, size_t, size_t&)> & fun) {
      return sig_add_org_birth.AddAction(fun);
    }

    void Clear() {
      if (sig_clear.GetNumActions()) sig_clear.Trigger();
      else base_t::Clear();  // If no actions are linked to sig_clear, use default.
    }

    void Update() { sig_update.Trigger(); }

    size_t AddOrg(ORG * new_org) {
      size_t new_pos;
      sig_add_org.Trigger(new_org, new_pos);
      return new_pos;
    }
    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      size_t offspring_pos;
      sig_add_org_birth.Trigger(new_org, parent_pos, offspring_pos);
      return offspring_pos;
    }
  };

  // A standard population manager for using synchronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_EA : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::fitM;

    emp::vector<ORG *> next_pop;

  public:
    PopulationManager_EA(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm) { ; }
    ~PopulationManager_EA() { base_t::Clear(); ClearNext(); }

    static constexpr bool emp_has_separate_generations = true;

    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      const size_t pos = next_pop.size();
      next_pop.push_back(new_org);
      return pos;
    }

    void ClearNext() {
      for (ORG * m : next_pop) if (m) delete m;
      next_pop.resize(0);
    }

    void Update() {
      base_t::SetOrgs(next_pop);  // Move over the next generation.
      next_pop.resize(0);         // Clear out the next pop to refill again.
    }
  };


  // A standard population manager for using a serial-transfer protocol.  All new
  // organisms get inserted into the main population; once it is full the population
  // is shrunk down.

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_SerialTransfer : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::fitM;

    size_t max_size;
    size_t bottleneck_size;
    size_t num_bottlenecks;
  public:
    PopulationManager_SerialTransfer(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm)
    , max_size(1000), bottleneck_size(100), num_bottlenecks(0) { ; }
    ~PopulationManager_SerialTransfer() { ; }

    size_t GetMaxSize() const { return max_size; }
    size_t GetBottleneckSize() const { return bottleneck_size; }
    size_t GetNumBottlnecks() const { return num_bottlenecks; }

    void SetMaxSize(const size_t m) { max_size = m; }
    void SetBottleneckSize(const size_t b) { bottleneck_size = b; }

    void ConfigPop(size_t m, size_t b) { max_size = m; bottleneck_size = b; }

    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      if (base_t::GetSize() >= max_size) {
        base_t::DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      return base_t::AddOrgAppend(new_org);
    }
  };

  template <typename ORG=int, typename FIT_MANAGER=int>
  class PopulationManager_Grid : public PopulationManager_Base<ORG,FIT_MANAGER> {
  protected:
    using base_t = PopulationManager_Base<ORG,FIT_MANAGER>;
    using base_t::fitM;
    using base_t::random_ptr;

    size_t width;
    size_t height;

  public:
    PopulationManager_Grid(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm) {
      ConfigPop(10,10);
    }
    ~PopulationManager_Grid() { ; }

    size_t GetWidth() const { return width; }
    size_t GetHeight() const { return height; }
    size_t GetRandomNeighbor(size_t id) const {
      const int offset = random_ptr->GetInt(9);
      const int rand_x = (int) (id%width) + offset%3 - 1;
      const int rand_y = (int) (id/width) + offset/3 - 1;
      return (size_t) (emp::mod(rand_x, (int) width) + emp::mod(rand_y, (int) height) * (int)width);
    }

    void ConfigPop(size_t w, size_t h) { width = w; height = h; base_t::Resize(width*height); }

    // Injected orgs go into a random position.
    size_t AddOrg(ORG * new_org) {
      emp::vector<size_t> empty_spots = base_t::GetEmptyPopIDs();
      const size_t pos = (empty_spots.size()) ?
        empty_spots[ random_ptr->GetUInt(empty_spots.size()) ] :
        random_ptr->GetUInt(base_t::GetSize());
      return base_t::AddOrgAt(new_org, pos);
    }

    // Newly born orgs go next to their parents.
    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      return base_t::AddOrgAt(new_org, GetRandomNeighbor(parent_pos));
    }

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream& os=std::cout,
               const std::string & empty="-", const std::string & spacer=" ") {
      emp_assert(string_fun);
      for (size_t y=0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
          ORG * org = base_t::GetOrg(x+y*width);
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
    using base_t::num_orgs;

    size_t pool_count;                             // How many pools are in the population?
    vector<size_t> pool_sizes;                     // How large is each pool?
    std::map<size_t, vector<size_t> > connections; // Which other pools can each position access?
    vector<size_t> pool_end;                       // Where does the next pool begin? First begins at 0.
    double mig_rate;                               // How often do organisms migrate to a connected pool?
    vector<size_t> pool_id;

  public:
    PopulationManager_Pools(const std::string & _w_name, FIT_MANAGER & _fm)
      : base_t(_w_name, _fm) {;}
    ~PopulationManager_Pools() {;}

    size_t GetPoolCount() const { return pool_count; }
    const vector<size_t> & GetSizes() const { return pool_sizes ; }

    void Setup(Random * r) {
      base_t::SetRandom(r);
      vector<size_t> * temp_sizes = new vector<size_t>;
      std::map<size_t, vector<size_t> > temp_connect;

      ConfigPop(5, *temp_sizes, &temp_connect, 150, 10, 0.05, 200);
    }

    // Sets up population based on user specs.
    void ConfigPop(size_t _pc, vector<size_t> _ps, std::map<size_t, vector<size_t> > * _c, double _mg, size_t _pop_size) {
      pool_count = _pc;
      pool_sizes = _ps;
      connections = *_c;
      mig_rate = _mg;
      pool_end = {};

      base_t::Resize(_pop_size);
      pool_id.resize(_pop_size, 0);

      // If no pool sizes in vector, error out
      if (pool_sizes.size() == 0) {
          std::cerr << "ERROR: No Pool sizes specified" <<std::endl;
          return;
      }
      // If only one pool size in vector, uses that size for all pools
      else if (pool_sizes.size() == 1) {
        size_t temp = pool_sizes[0];
        for (size_t i = 1; i < pool_count; i++) { pool_sizes.push_back(temp); }
      }
      // If not enough pool sizes given, error out
      else if (pool_sizes.size() != pool_count) {
        std::cerr << " ERROR: Not enough pool sizes" << std::endl;
        return;
      }

      size_t total = 0;
      for (auto el : pool_sizes) { total += el; }

      emp_assert(_pop_size == total && "POP_SIZE must equal total pool sizes");

      // Divide World into pools
      size_t arr_size = 0;
      size_t prev_size = 0;
      size_t pool_num = 0;
      for (auto el : pool_sizes) {
        arr_size += el;
        for( size_t i = prev_size; i < arr_size; i++) {
          pool_id[i] = pool_num;
        }
        prev_size = arr_size;
        pool_num++;
        pool_end.push_back(arr_size);
      }
      return;
    }

    // Injected orgs go into a random pool.
    size_t AddOrg(ORG * new_org) {
      Range<size_t> id_range(0, pop.size);

      // If any pools are empty, choose org from the next pool.
      if (num_orgs < pool_count) {
        id_range.upper = pool_end[num_orgs];
        if (num_orgs > 0) { id_range.lower = pool_end[num_orgs-1]; }
      }

      const size_t pos = random_ptr->GetInt(id_range);
      return AddOrgAt(new_org, pos);
    }

    // Newly born orgs have a chance to migrate to a connected pool.
    size_t AddOrgBirth(ORG * new_org, size_t parent_pos) {
      size_t InsertPool = 0;  // Which pool should new org be born into?

      // Test if a migration should happen ; if so, determine new pool.
      const auto & parent_conns = connections[parent_pos];
      if (random_ptr->P(mig_rate) && parent_conns.size() > 0) {
        size_t conn_id = random_ptr->GetInt(0, parent_conns.size());
        InsertPool = parent_conns[conn_id];
      }
      else { InsertPool = pool_id[parent_pos]; }

      size_t range_l = InsertPool ? pool_end[InsertPool-1] : 0;
      size_t range_u = pool_end[InsertPool];

      const size_t pos = random_ptr->GetInt(range_l, range_u);
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
