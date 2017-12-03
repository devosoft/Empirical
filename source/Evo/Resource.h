#ifndef EMP_EVO_RESOURCE_H
#define EMP_EVO_RESOURCE_H

#include "World.h"

namespace emp {

    class Resource {
    private:
        double amount = 0;
        double inflow = 0;
        double outflow = 0;

    public:

        Resource(){};
        Resource(double amt, double in, double out) :
            amount(amt), inflow(in), outflow(out){};

        double GetAmount() {return amount;}
        double GetInflow() {return inflow;}
        double GetOutflow() {return outflow;}

        void SetAmount(double amt) {amount = amt;}
        void SetInflow(double in) {inflow = in;}
        void SetOutflow(double out) {outflow = out;}

        double Inc() {amount += inflow; return amount;}
        double Inc(double amt) {amount += amt; return amount;}
        double Dec() {
            amount -= amount*outflow;
            if (amount < 0) {amount = 0;}
            return amount;
        }
        double Dec(double amt) {
            amount -= amt;
            if (amount < 0) {amount = 0;}
            return amount;
        }
        double Update() {
            amount += inflow - (amount*outflow);
            if (amount < 0) {amount = 0;}
            return amount;
        }
    };

    template <typename ORG>
    void ResourceSelect(World<ORG> & world, const emp::vector<std::function<double(const ORG &)> > & extra_funs,
                   emp::vector<emp::Resource> & pools, size_t t_size, size_t tourny_count=1, double frac = .0025, double max_bonus = 5) {

       emp_assert(world.GetFitFun(), "Must define a base fitness function");
       emp_assert(world.GetSize() > 0);
       emp_assert(t_size > 0 && t_size <= world.GetSize(), t_size, world.GetSize());
       emp_assert(world.IsCacheOn() == false, "Ecologies mean constantly changing fitness!");

       // Setup info to track fitnesses.
       emp::vector<double> base_fitness(world.GetSize());
       emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
       for (size_t i=0; i < extra_funs.size(); i++) {
         extra_fitnesses[i].resize(world.GetSize());
       }

       // Collect all fitness info.
    //    std::cout << extra_funs.size() << std::endl;
       for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
         base_fitness[org_id] = world.CalcFitnessID(org_id);
         for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
            //  std::cout << "Test" << std::endl;
           double cur_fit = emp::Pow(extra_funs[ex_id](world[org_id]), 2.0);
           cur_fit *= frac*pools[ex_id].GetAmount();
           cur_fit = std::min(cur_fit, max_bonus);
           extra_fitnesses[ex_id][org_id] = emp::Pow(2.0,cur_fit);
        //    std::cout << "Fit before:  = " << base_fitness[org_id] << "   Res: " << pools[ex_id].GetAmount();
           base_fitness[org_id] *= emp::Pow(2.0,cur_fit);
           pools[ex_id].Dec(cur_fit);
        //    std::cout << "   Bonus " << ex_id << " = " << extra_funs[ex_id](world[org_id]) << " "<< emp::Pow(2.0,cur_fit)
        //              << "   fitnes = " << base_fitness[org_id]
        //              << std::endl;

         }
       }

    //    std::cout << "Resource allocations" << std::endl;
    //    std::cout << emp::to_string(base_fitness) << std::endl;
    //    for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
    //
    //    }

       emp::vector<size_t> entries;
       for (size_t T = 0; T < tourny_count; T++) {
         entries.resize(0);
         for (size_t i=0; i<t_size; i++) entries.push_back( world.GetRandomOrgID() ); // Allows replacement!

         double best_fit = base_fitness[entries[0]];
         size_t best_id = entries[0];

         // Search for a higher fit org in the tournament.
         for (size_t i = 1; i < t_size; i++) {
           const double cur_fit = base_fitness[entries[i]];
           if (cur_fit > best_fit) {
             best_fit = cur_fit;
             best_id = entries[i];
           }
         }

         // Place the highest fitness into the next generation!
         world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
       }


    }

}

#endif
