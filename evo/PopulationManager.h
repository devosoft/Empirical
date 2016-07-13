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

  template <typename ORG=int>
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

    void SetRandom(Random * r) { random_ptr = r; }


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
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
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

    int width;
    int height;

    int ToX(int id) const { return id % width; }
    int ToY(int id) const { return id / width; }
    int ToID(int x, int y) const { return y*width + x; }

  public:
    PopulationManager_Grid() { ConfigPop(10,10); }
    ~PopulationManager_Grid() { ; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

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
 class PopulationManager_Pools : public PopulationManager_Base<ORG> {
  public:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::SetRandom;
    using PopulationManager_Base<ORG>::GetSize;

    int pool_count;
    vector<int> pool_sizes;
    std::map<int, vector<int> > connections; //connected pool numbers, starting at 0.
    int org_count = 0; // orgs in vector
    int r_upper; //random upper limit
    int r_lower;// random lower limit
    vector<int> pool_end; //end of each pool in array
    double mig_rate;

  public:
    PopulationManager_Pools() { ; }
    ~PopulationManager_Pools() { ; }

    int GetPoolCount() const { return pool_count; }
    vector<int> GetSizes() const { return pool_sizes ; }
    int GetUpper() const { return r_upper; }
    int GetLower() const { return r_lower; }

    void Setup(Random * r){
        vector<int>* temp_sizes = new vector<int>;
        std::map<int, vector<int> > temp_connect;
        for(int i = 0; i < 5; i++){ temp_connect[i].resize(0); }

        SetRandom(r);
        ConfigPop(5, *temp_sizes, &temp_connect, 150, 10, 0.05, 200);
    }

    //Sets up population based on user specs.
    void ConfigPop(int pc, vector<int> ps, std::map<int, vector<int> > * c, int u, int l, double mg, int pop_size) { 
        pool_count = pc; 
        pool_sizes = ps;
        r_upper = u;
        r_lower = l;
        connections = *c;
        mig_rate = mg;

        pop.resize(pop_size, nullptr);

        // If no pool sizes in vector, defaults to random sizes for each
        if(pool_sizes.size() == 0){
            while(true){ 
                int pool_total = 0;
                for( int i = 0; i < pool_count - 1; i++){
                    pool_sizes.push_back(random_ptr->GetInt(r_lower, r_upper));
                    pool_total += pool_sizes[i];
                }

                if(pool_total < pop_size){ //Keep generating random sizes until true
                    pool_sizes.push_back(pop_size - pool_total);
                    break;
                }

                for( int i = 0; i < pool_count - 1; i++ ){pool_sizes.pop_back();}
            }
        }
        // If only one pool size in vector, uses that size for all pools
        else if(pool_sizes.size() == 1){
            int temp = pool_sizes[0];
            for(int i = 1; i < pool_count; i++){ pool_sizes.push_back(temp); }
        }
        else if(pool_sizes.size() != pool_count){
            std::cerr<<" ERROR: Not enough pool sizes"<<std::endl;
            return;
        }

        int total = 0;
        for (auto el : pool_sizes){total += el;}

        emp_assert(pop_size == total && "POP_SIZE is different than total pool sizes");

        // Divide World into pools
        int arr_size = 0;
        for( auto el : pool_sizes ){
            arr_size += el;
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

        if(random_ptr->P(mig_rate) && connections[parent_pos].size() > 0){
            int loc = random_ptr->GetInt(0, connections[parent_pos].size());

            vector<int> temp  = connections[parent_pos];
            InsertPool = temp[loc];

            std::cout<<"Crossing To: "<<InsertPool<<std::endl;
        }
        else{
            for(int i = 0; i < pool_end.size(); i++ ){
                if(parent_pos < pool_end[i]) {InsertPool = i; break;} 
            }
        }

        if(InsertPool == 0){range_l = 0;}
        else{range_l = pool_end[InsertPool - 1];}
        range_u = pool_end[InsertPool];

        const int pos = (int) random_ptr->GetDouble(range_l, range_u);

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
 };


  using PopBasic = PopulationManager_Base<int>;
  using PopEA    = PopulationManager_EA<int>;
  using PopST    = PopulationManager_SerialTransfer<int>;
  using PopGrid  = PopulationManager_Grid<int>;
  using PopPool  = PopulationManager_Pools<int>;
  using PopGPool = PopulationManager_GridPools<int>;

}
}


#endif
