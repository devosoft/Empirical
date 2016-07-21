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

#include <set>
#include <array>
#include "../tools/random_utils.h"
#include "PopulationIterator.h"

namespace emp {
namespace evo {

  template <typename POP_MANAGER> class PopulationIterator;

  template <typename ORG=int>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    emp::vector<ORG *> pop;

    unsigned int curr_ins_index;
    Random * random_ptr;

  public:
    PopulationManager_Base() { ; }
    ~PopulationManager_Base() { ; }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ORG*;

    friend class PopulationIterator<PopulationManager_Base<ORG> >;
    using iterator = PopulationIterator<PopulationManager_Base<ORG> >;

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }
    iterator begin(){return iterator(this, 0);}
    iterator end(){return iterator(this, pop.size());}

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }
    int GetSize() const { return (int) pop.size(); }
    ORG const * GetPos(size_t pos) { return pop[pos];} 

    void SetRandom(Random * r) { random_ptr = r; }
    unsigned int SequentialInsert(ORG * org) { 
      pop.push_back(org);
      return pop.size() - 1;
    }

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

    // AddOrg and ReplaceOrg should be the only ways new organisms come into a population.
    // AddOrg inserts them into the end of the designated population.
    // ReplaceOrg places them at a specific position, replacing anyone who may already be there.
    int AddOrg(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
    int AddOrgBirth(ORG * new_org, int parent_pos) {
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
        if (m) m->Execute(std::forward<ARGS>(args)...);
      }
    }


    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const int new_size, bool choose_random=true) {
      if (new_size >= (int) pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Delete all of the organisms we are removing and resize the population.
      for (int i = new_size; i < (int) pop.size(); ++i) delete pop[i];
      pop.resize(new_size);
    }
  };

  // A standard population manager for using synchronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename ORG=int>
  class PopulationManager_EA : public PopulationManager_Base<ORG> {
  protected:
    emp::vector<ORG *> next_pop;
    using PopulationManager_Base<ORG>::pop;

  public:
    PopulationManager_EA() { ; }
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
    }

    void Update() {
      for (ORG * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                 // Move over the next generation.
      next_pop.resize(0);             // Clear out the next pop to refill again.
    }
  };


  // A standard population manager for using a serial-transfer protocol.  All new
  // organisms get inserted into the main population; once it is full the population
  // is shrunk down.

  template <typename ORG=int>
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

    void ConfigPop(int m, int b) { max_size = m; bottleneck_size = b; }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      if (pop.size() >= max_size) {
        DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
  };

  template <typename ORG=int>
  class PopulationManager_Grid : public PopulationManager_Base<ORG> {
  protected:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::curr_ins_index;

    int width;
    int height;

    int ToX(int id) const { return id % width; }
    int ToY(int id) const { return id / width; }
    int ToID(int x, int y) const { return y*width + x; }

  public:
    static int GetMaxPossibleInRadius(ORG * org, int radius) { return radius * radius;}
    
    PopulationManager_Grid() { 
      ConfigPop(10,10);
      curr_ins_index = 0;
    }
    ~PopulationManager_Grid() { ; }

    // this method will insert organisms sequentially through the population--used for 
    // initialization. Automatically wraps insertion point
    unsigned int SequentialInsert(ORG * org) {
      int point = curr_ins_index;
      pop[curr_ins_index++] = org;
      curr_ins_index %= (width * height);
      return point;
    }

    std::set<ORG *>  DoBottleneckEvent(double lethality) {
      std::set<ORG *> deck;

      for(size_t i = 0; i < pop.size(); i++) {
        if(random_ptr->P(lethality)) {
          if(pop[i] != nullptr) {
            delete pop[i];
            pop[i] = nullptr;
          }
        }
        else {
          deck.insert(pop[i]);
          pop[i] = nullptr;
        }
      }

      return deck;
    }


    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // method to get all the von-neuman (sp?) neighbors of a particular organism
    // does not include the organism itself
    std::set<ORG *> GetOrgNeighbors (int org_id) {
      std::set<ORG *> neighbors;
      int org_x, org_y;
      org_x = ToX(org_id);
      org_y = ToY(org_id);

      // iterate over all possible spaces && add organisms to set
      // j
      // using the set will prevent duplictes, since we *WILL* traverse the same spaces
      // multiple times.
      for(int i = -1; i < 2; i++) {
        for(int j = -1; j < 2; j++) {
          neighbors.insert(pop[ToID((org_x + i + width) % width, 
                                    (org_y + j + height) % height)]);
        }
      }

      neighbors.erase(neighbors.find(pop[org_id])); // remove focal node from set
      return neighbors;
    }

    //TODO@JGF: a) make the rest of my todo's @'d to me
    //          b) make the function naming consistent (e.g. camel, not _'s)


    std::set<ORG *> GetClusterByRadius(unsigned int focal_id, int depth) {
      std::set<unsigned int> explored;

      unsigned int org_x, org_y, target;

      // execute a breadth-first-search, depth nodes deep, from focal_id
      

      org_x = ToX(focal_id);
      org_y = ToY(focal_id);
      for(int i = -(depth / 2); i < (depth / 2); i++) {
        for(int j = -(depth / 2); j < (depth / 2); j++){
          target = ToID((org_x + i + width) % width, (org_y + j + height) % height);
          explored.insert(target);
        }
      }


      std::set<ORG *> orgs;
      for (auto site : explored) {
        if (pop[site] != nullptr) { orgs.insert(pop[site]); }
      }
      return orgs;
    }

    void ConfigPop(int w, int h) { width = w; height = h; pop.resize(width*height, nullptr); }

    // Injected orgs go into a random position.
    int AddOrg(ORG * new_org) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
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

      return pos;
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

 template <typename ORG>
 class PopulationManager_MixedGrid : public PopulationManager_Grid<ORG> {
  public:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::SetRandom;
    using PopulationManager_Base<ORG>::GetSize;

    using PopulationManager_Grid<ORG>::ConfigPop;
    using PopulationManager_Grid<ORG>::curr_ins_index;
    using PopulationManager_Grid<ORG>::ToX;
    using PopulationManager_Grid<ORG>::ToY;
    using PopulationManager_Grid<ORG>::ToID;
    using PopulationManager_Grid<ORG>::width;
    using PopulationManager_Grid<ORG>::height;
    using PopulationManager_Grid<ORG>::DoBottleneckEvent;


    PopulationManager_MixedGrid() { 
      ConfigPop(10,10);
      curr_ins_index = 0;
    }

    ~PopulationManager_MixedGrid() { ; }

    // override the add-org-birth method to randomly assign a location
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = random_ptr->GetInt((int) pop.size());
     
      if (pop[pos] != nullptr) delete pop[pos];
      pop[pos] = new_org;

      return pos;
    }
    
 };

  template <typename ORG>
  class PopulationManager_Pools : public PopulationManager_Base<ORG> {
  public:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::SetRandom;
    using PopulationManager_Base<ORG>::GetSize;

    int pool_count;                           // How many pools are in the population?
    vector<int> pool_sizes;                   // How large is each pool?
    std::map<int, vector<int> > connections;  // Which other pools can each position access?

    int org_count;                            // orgs in vector              @CAO: What vector?
    int r_upper;                              // random upper limit          @CAO: Limit of what?
    int r_lower;                              // random lower limit
    vector<int> pool_end;                     // end of each pool in array   @CAO: Last element?
    double mig_rate;

  public:
    PopulationManager_Pools() : org_count(0) { ; }
    ~PopulationManager_Pools() { ; }

    int GetPoolCount() const { return pool_count; }
    const vector<int> & GetSizes() const { return pool_sizes ; }
    int GetUpper() const { return r_upper; }
    int GetLower() const { return r_lower; }

    void Setup(Random * r) {
      vector<int>* temp_sizes = new vector<int>;
      std::map<int, vector<int> > temp_connect;
      for (int i = 0; i < 5; i++) { temp_connect[i].resize(0); }  // @CAO: Why 5?  What is it?

      SetRandom(r);
      ConfigPop(5, *temp_sizes, &temp_connect, 150, 10, 0.05, 200);
    }

    //Sets up population based on user specs.
    void ConfigPop(int pc, vector<int> ps, std::map<int, vector<int> > * c, int u, int l,
                   double mg, int pop_size) {
      pool_count = pc;
      pool_sizes = ps;
      r_upper = u;
      r_lower = l;
      connections = *c;
      mig_rate = mg;

      pop.resize(pop_size, nullptr);

      // If no pool sizes in vector, defaults to random sizes for each
      if (pool_sizes.size() == 0) {
        while (true) {
          int pool_total = 0;
          for( int i = 0; i < pool_count - 1; i++){
            pool_sizes.push_back(random_ptr->GetInt(r_lower, r_upper));
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
      for (auto el : pool_sizes) {
        arr_size += el;
        pool_end.push_back(arr_size);
      }

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

      const int pos = (int) random_ptr->GetDouble(range_l, range_u);

      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      org_count++;

      return pos;
    }

    // Newly born orgs have a chance to migrate to a connected pool.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      int insert_pool = 0;  // Which pool should new org be born into?

      // Test if a migration should happen ; if so, determine new pool.
      const auto & parent_conns = connections[parent_pos];
      if (random_ptr->P(mig_rate) && parent_conns.size() > 0) {
        int conn_id = random_ptr->GetInt(0, parent_conns.size());
        insert_pool = parent_conns[conn_id];

        std::cout << "Crossing To: " << insert_pool << std::endl;  // @CAO: Remove?
      }
      else {
        // @CAO: The below is inefficient.  Keep a chart like connections to look up in const time.
        for (int i = 0; i < pool_end.size(); i++) {
          if (parent_pos < pool_end[i]) { insert_pool = i; break; }
        }
      }

      int range_l = insert_pool ? pool_end[insert_pool-1] : 0;
      int range_u = pool_end[insert_pool];

      const int pos = (int) random_ptr->GetDouble(range_l, range_u);  // @CAO Why GetDouble not GetInt?
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;

      return pos;
    }
 };

  template <typename ORG>
 class PopulationManager_GridPools : public PopulationManager_Base<ORG> {
  public:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::SetRandom;
    using PopulationManager_Base<ORG>::GetSize;

    int pool_count;
    int width;
    int height;
    int tot_pop;
    int org_count = 0; // orgs in vector
    vector<int> pool_end; //end of each pool in array

    int ToX(int id) const { return id % width; }
    int ToY(int id) const { return id / width; }
    int ToID(int x, int y) const { return y*width + x; }

  public:
    PopulationManager_GridPools() { ; }
    ~PopulationManager_GridPools() { ; }

    int GetPoolCount() const { return pool_count; }
    int GetMaxPossibleInRadius (ORG * ptr, int radius) {
      return (width * height > radius * radius) ? width * height : radius * radius;
    }

    void Setup(Random * r){

        SetRandom(r);
        ConfigPop(5, 4, 5, 100);
    }

    //Sets up population based on user specs.
    void ConfigPop(int pc, int w, int h, int pop_size) { 
        pool_count = pc; 
        width = w;
        height = h;
        tot_pop = pop_size;

        pop.resize(width * height * pool_count, nullptr);

        // Divide World into pools
        int arr_size = 0;
        for( int i = 0 ; i < pool_count; i++){
            arr_size += width * height;
            pool_end.push_back(arr_size);
        }
    
    }

    // Injected orgs go into a random pool.
    int AddOrg(ORG * new_org) {
        int range_u;
        int range_l = 0;

        //Each pool is guarenteed to have 1 org.
        if(org_count < pool_count){
            range_u = pool_end[org_count];
            if(org_count > 0){range_l = pool_end[org_count - 1];}
        }
        else{
            range_u = (int) pop.size();
        }

        const int pos = (int) random_ptr->GetDouble(range_l, range_u);
         
        if (pop[pos]) delete pop[pos];
        pop[pos] = new_org;
        org_count++;

        return pos;
    }

    // Newly born orgs have a chance to migrate to a connected pool.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
        int InsertPool = 0;
        int range_l, range_u;

        for(int i = 0; i < pool_end.size(); i++ ){
            if(parent_pos < pool_end[i]) {InsertPool = i; break;} 
        }

        if(InsertPool == 0){range_l = 0;}
        else{range_l = pool_end[InsertPool - 1];}
        range_u = pool_end[InsertPool];

        const int parent_x = ToX(parent_pos- range_l);
        const int parent_y = ToY(parent_pos - range_l);
        const int offset = random_ptr->GetInt(9);
        const int offspring_x = emp::mod(parent_x + offset%3 - 1, width);
        const int offspring_y = emp::mod(parent_y + offset/3 - 1, height);
        const int pos = ToID(offspring_x, offspring_y) + range_l;

        if (pop[pos]) delete pop[pos];

        pop[pos] = new_org;

        return pos;
    }

    int ToSubX(int id) { return id % (width * height) % width;}
    int ToSubY(int id) { return id % (width * height) / width;}
    int ToSubID(int x, int y) {return y*width + x;}

    std::set<ORG *> GetOrgNeighbors (int org_id) {
      std::set<ORG *> neighbors;
      int org_sx = ToSubX(org_id);
      int org_sy = ToSubY(org_id);
      int org_grid = org_id / (width * height);
      
      for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
          neighbors.insert(pop[ToID((org_sx + i + width) % width,
                                    (org_sy + j + height) % height) + org_grid * width * height]);
        }
      }

      // remove the focal node
      neighbors.erase(neighbors.find(pop[org_id]));
      return neighbors; 
    }


    std::set<ORG *> DoBottleneckEvent(double lethality) {
      // get complement of lethality--will tell us how many to save
      double immune = 1.0 - lethality;
      unsigned int num_immune = tot_pop * immune;

      vector<ORG *> choices;
      std::set<ORG *> chosen;
      // build a list of non-null organisms
      for (auto org : pop) { if (org) choices.push_back(org);}

      // impossible to get sufficient organisms to meet bottleneck requirement
      // grab as many as we can and return them
      if (choices.size() < num_immune) {
        for (auto org : choices) {chosen.insert(org);}
        choices.resize(0);
      }
      else {
        // otherwise, build a collection of organisms of the specified size,
        // randomly chosen from available organisms
        while (chosen.size() < num_immune) {
          size_t choice = random_ptr->GetUInt(0, choices.size());

          // insert the new org into the 'next' pool
          // then shrink the size of the choice pool
          chosen.insert(choices[choice]);
          choices[choice] = choices[choices.size() - 1];
          choices.pop_back();
        }
      }

      // everything left in 'choices' should now be killed
      for (auto org : choices) {delete org;}

      // population can be reset
      // can't use the inherited 'clear' method, might delete things we still want
      for (size_t i = 0; i < pop.size(); i++) {pop[i] = nullptr;}
      // reseed 
      org_count = 0;
      //for (auto org : chosen) {org->set_id(AddOrg(org));}
      return chosen;
    }

    // get the organisms within a certian radius of another org
    std::set<ORG *> GetClusterByRadius(unsigned int focal_id, int depth) {
      // determine which subgrid we're on
      unsigned int grid = focal_id / (width * height);

      std::set<unsigned int> explored;

      unsigned int org_sx, org_sy, target;

      // execute a breadth-first-search, depth nodes deep, from focal_id
      

      org_sx = ToSubX(focal_id);
      org_sy = ToSubY(focal_id);
      for(int i = -(depth / 2); i < (depth / 2); i++) {
        for(int j = -(depth / 2); j < (depth / 2); j++){
          target = ToSubID((org_sx + i + width) % width, (org_sy + j + height) % height);
          explored.insert(target);
        }
      }


      std::set<ORG *> orgs;
      for (auto site : explored) {
        if (pop[site] != nullptr) { orgs.insert(pop[site + width * height * grid]); }
      }
      return orgs;
    }



 };


  using PopBasic = PopulationManager_Base<int>;
  using PopEA    = PopulationManager_EA<int>;
  using PopST    = PopulationManager_SerialTransfer<int>;
  using PopGrid  = PopulationManager_Grid<int>;
  using PopPool  = PopulationManager_Pools<int>;

}
}


#endif
