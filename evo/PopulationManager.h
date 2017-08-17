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
    int num_orgs;
    Random * random_ptr;

  private:
    pop_t pop;

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
    PopulationManager_Base(const std::string &, FIT_MANAGER & _fm) : fitM(_fm), num_orgs(0) { ; }
    ~PopulationManager_Base() { Clear(); }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ptr_t;

    using iterator_t = PopulationIterator<PopulationManager_Base<ORG,FIT_MANAGER> >;
    friend class interator_t;

    int GetSize() const { return (int) pop.size(); }
    int GetNumOrgs() const { return num_orgs; }
    ptr_t GetOrg(int id) const { return pop[id]; }
    double CalcFitness(int id, const std::function<double(ORG*)> & fit_fun) const {
      return fitM.CalcFitness(id, pop[id], fit_fun);
    }

    void SetRandom(Random * r) { random_ptr = r; }
    void Setup(Random * r) { SetRandom(r); }

    // AddOrgAt, AddOrgAppend, and SetOrgs are the only ways new organisms come into a population
    // (all others go through these)

    int AddOrgAt(ORG * new_org, int pos) {
      emp_assert(pos < (int) pop.size());   // Make sure we are placing into a legal position.
      if (pop[pos]) { delete pop[pos]; --num_orgs; }
      pop[pos] = new_org;
      fitM.ClearAt(pos);
      ++num_orgs;
      return pos;
    }

    int AddOrgAppend(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      fitM.ClearAt(pos);
      ++num_orgs;
      return pos;
    }

    virtual emp::vector<int> GetNeighbors(int t_size, int id) {
        emp::vector<int> neighbors;
        for(int i = 0; i < t_size; i++) neighbors.push_back(GetRandomOrg());
        return neighbors;
    }

    bool CheckValidOrg(int org_pos){
        if (pop[org_pos] == nullptr)
            return false;
        return true;
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

    void ClearOrgAt(int pos) {
      // Delete given organism.
      if (pop[pos]) { delete pop[pos]; pop[pos]=nullptr; num_orgs--; }  // Delete current organisms.
      fitM.ClearAt(pos);
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

    void Resize(size_t new_size) {
      emp_assert(new_size >= 0);
      for (int i = new_size; i < (int) pop.size(); i++) ClearOrgAt(i); // Remove orgs out or range.
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
    int GetRandomCell() const { return random_ptr->GetInt(0, pop.size()); }

    // By default, assume a well-mixed population so random neighbors can be anyone.
    int GetRandomNeighbor(int /*id*/) const { return random_ptr->GetInt(0, pop.size()); }

    // Get random *occupied* cell.
    int GetRandomOrg() const {
      emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
      int pos = random_ptr->GetInt(0, pop.size());
      //while (pop[pos] == nullptr) pos = random_ptr->GetInt(0, pop.size());
      return pos;
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

      // Clear out all of the organisms we are removing and resize the population.
      for (int i = new_size; i < (int) pop.size(); ++i) ClearOrgAt(i);
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

    // Most of the key functions in the population manager can be interfaced with symbols.  If you
    // need to modify the more complex behaviors (such as Execute) you need to create a new
    // derrived class from PopulationManager_Base, which is also legal in a plugin.
    Signal<> sig_clear;
    Signal<> sig_update;
    Signal<ORG*, int&> sig_add_org;            // args: new org, return: offspring pos
    Signal<ORG*, int, int&> sig_add_org_birth; // args: new org, parent pos, return: offspring pos

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
    LinkKey OnAddOrg(const std::function<void(ORG*, int&)> & fun) {
      return sig_add_org.AddAction(fun);
    }
    LinkKey OnAddOrgBirth(const std::function<void(ORG*, int, int&)> & fun) {
      return sig_add_org_birth.AddAction(fun);
    }

    void Clear() {
      if (sig_clear.GetNumActions()) sig_clear.Trigger();
      else base_t::Clear();  // If no actions are linked to sig_clear, use default.
    }

    void Update() { sig_update.Trigger(); }

    int AddOrg(ORG * new_org) {
      int new_pos;
      sig_add_org.Trigger(new_org, new_pos);
      return new_pos;
    }
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      int offspring_pos;
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

    void Setup(){ ; }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = next_pop.size();
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

    void Setup(Random *r){ }

    void SetMaxSize(const int m) { max_size = m; }
    void SetBottleneckSize(const int b) { bottleneck_size = b; }

    void ConfigPop(int m, int b) { max_size = m; bottleneck_size = b; }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
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

    int width;
    int height;

  public:
    using base_t::CheckValidOrg;
    PopulationManager_Grid(const std::string & _w_name, FIT_MANAGER & _fm)
    : base_t(_w_name, _fm) {
      ConfigPop(10,10);
    }
    ~PopulationManager_Grid() { ; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    int GetRandomNeighbor(int id) const {
      const int offset = random_ptr->GetInt(9);
      const int rand_x = emp::mod(id%width + offset%3 - 1, width);
      const int rand_y = emp::mod(id/width + offset/3 - 1, height);
      return rand_x + rand_y*width;
    }

    emp::vector<int> GetNeighbors(int t_size, int id){
        emp::vector<int> neighbors;
        for (int offset = 0; offset < t_size; offset++){
            int x_pos = emp::mod(id%width + offset%3 - 1, width);
            int y_pos = emp::mod(id/width + offset/3 - 1, height);
            int pos = x_pos + y_pos*width;
            //if(CheckValidOrg(pos))
                    //neighbors.push_back(pos);
            neighbors.push_back(pos);
        }
        return neighbors;
    }

    void ConfigPop(int w, int h) { width = w; height = h; base_t::Resize(width*height); }

    // Injected orgs go into a random position.
    int AddOrg(ORG * new_org) {
      emp::vector<int> empty_spots = base_t::GetEmptyPopIDs();
      const int pos = (empty_spots.size()) ?
        empty_spots[ random_ptr->GetUInt(empty_spots.size()) ] :
        random_ptr->GetUInt(base_t::GetSize());
      return base_t::AddOrgAt(new_org, pos);
    }

    // Newly born orgs go next to their parents.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      return base_t::AddOrgAt(new_org, parent_pos);
    }

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream& os=std::cout,
               const std::string & empty="-", const std::string & spacer=" ") {
      emp_assert(string_fun);
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
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

    int pool_count;                            // How many pools are in the population?
    vector<int> pool_sizes;                    // How large is each pool?
    std::map<int, vector<int> > connections;   // Which other pools can each position access?
    vector<int> pool_end;                      // Where does the next pool begin? First begins at 0.
    double mig_rate;                           // How often do organisms migrate to a connected pool?

    vector<int> pool_id;

  public:
    PopulationManager_Pools(const std::string & _w_name, FIT_MANAGER & _fm)
      : base_t(_w_name, _fm) {;}
    ~PopulationManager_Pools() {;}

    int GetPoolCount() const { return pool_count; }
    const vector<int> & GetSizes() const { return pool_sizes ; }

    void Setup(Random * r) {
      base_t::SetRandom(r);
      vector<int>* temp_sizes = new vector<int>;
      std::map<int, vector<int> > temp_connect;

      ConfigPop(5, *temp_sizes, &temp_connect, 150, 10, 0.05, 200);
    }

    // Sets up population based on user specs.
    void ConfigPop(int _pc, vector<int> _ps, std::map<int, vector<int> > * _c, double _mg, int _pop_size) {
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
        int temp = pool_sizes[0];
        for (int i = 1; i < pool_count; i++) { pool_sizes.push_back(temp); }
      }
      // If not enough pool sizes given, error out
      else if (pool_sizes.size() != pool_count) {
        std::cerr << " ERROR: Not enough pool sizes" << std::endl;
        return;
      }

      int total = 0;
      for (auto el : pool_sizes) { total += el; }

      emp_assert(_pop_size == total && "POP_SIZE must equal total pool sizes");

      // Divide World into pools
      int arr_size = 0;
      int prev_size = 0;
      int pool_num = 0;
      for (auto el : pool_sizes) {
        arr_size += el;
        for( int i = prev_size; i < arr_size; i++) {
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
      Range<int> id_range(0, pop.size);

      // If any pools are empty, choose org from the next pool.
      if (num_orgs < pool_count) {
        id_range.upper = pool_end[num_orgs];
        if (num_orgs > 0) { id_range.lower = pool_end[num_orgs-1]; }
      }

      const int pos = random_ptr->GetInt(id_range);
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
      else { InsertPool = pool_id[parent_pos]; }

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
