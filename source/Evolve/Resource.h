/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Resource.h
 *  @brief Implement resource-based selection.
 *
 *
 *  @todo Ultimately, we probably want a much more full-featured resource system.
 *        This one works for Eco-EA and could be the basis for something Avida-like
 *        but lacks a lot of features for Artificial Life systems.
 */


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
    void ResourceSelect(World<ORG> & world, emp::vector< std::function<double(ORG &)> > & extra_funs,
                   emp::vector<emp::Resource> & pools, size_t t_size, size_t tourny_count=1, double frac = .0025, double max_bonus = 5, double cost = 0, bool use_base = true, double min_score = 0) {

       emp_assert(world.GetFitFun(), "Must define a base fitness function");
       emp_assert(world.GetSize() > 0);
       emp_assert(t_size > 0, t_size);
    //    emp_assert(world.IsCacheOn() == false, "Ecologies mean constantly changing fitness!");

       // Setup info to track fitnesses.
       emp::vector<double> base_fitness(world.GetSize());
       emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
       for (size_t i=0; i < extra_funs.size(); i++) {
         extra_fitnesses[i].resize(world.GetSize());
       }

       // Collect all fitness info.
    //    std::cout << extra_funs.size() << std::endl;

       for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
        //    std::cout << org_id << std::endl;
         if (!world.IsOccupied(org_id)) {
             continue;
         }
        //  std::cout << "still going" << std::endl;
         if (use_base) {
            base_fitness[org_id] = world.CalcFitnessID(org_id);
         } else {
            base_fitness[org_id] = 0;
         }

         for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
            //  std::cout << "Test " << ex_id << std::endl;

           pools[ex_id].Inc(pools[ex_id].GetInflow()/world.GetNumOrgs());
           double cur_fit = extra_funs[ex_id](world.GetOrg(org_id));
           cur_fit = emp::Pow(cur_fit, 2.0);
            //    if (org_id==0) {std::cout << "Allele: " << world[org_id][ex_id] <<" Curr fit: " << extra_funs[ex_id](world[org_id]) << " Curr fit squared: " << cur_fit << " Amount: " << pools[ex_id].GetAmount() << " Frac: " << frac;}
            cur_fit *= frac*(pools[ex_id].GetAmount()-cost);
            if (cur_fit > min_score) {
                cur_fit -= cost;
            } else {
                cur_fit = 0;
            }
        //    if (org_id==0) {std::cout << " Multiplied out: " << cur_fit;}
           cur_fit = std::min(cur_fit, max_bonus);
        //    if (org_id==0) {std::cout << " Final: " << cur_fit << std::endl;}
           extra_fitnesses[ex_id][org_id] = emp::Pow2(cur_fit);
        //    std::cout << "Fit before:  = " << base_fitness[org_id] << "   Res: " << pools[ex_id].GetAmount();
           base_fitness[org_id] *= emp::Pow2(cur_fit);
           pools[ex_id].Dec(std::abs(cur_fit));
        //    std::cout << "   Bonus " << ex_id << " = " << extra_funs[ex_id](world[org_id]) << " "<< emp::Pow(2.0,cur_fit) << " " << emp::to_string(world[org_id])
        // //              << "   fitnes = " << base_fitness[org_id]
        //              << std::endl;

         }
       }

    //    std::cout << "Resource allocations" << std::endl;
    //    std::cout << emp::to_string(base_fitness) << std::endl;
    //    std::cout << emp::to_string(world[0]) << std::endl;
    //    std::cout << world.CalcFitnessID(0);

    //    for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
    //        std::cout << extra_fitnesses[ex_id][0] << " ";
    //    }
    //    std::cout << std::endl;

       emp::vector<size_t> entries;
       for (size_t T = 0; T < tourny_count; T++) {
         entries.resize(0);
        //  std::cout << T << std::endl;
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

        // IndexMap fitness_index(world.GetSize());
        // for (size_t id = 0; id < world.GetSize(); id++) {
        //   fitness_index.Adjust(id, base_fitness[id]);
        // }
        //
        // for (size_t n = 0; n < tourny_count; n++) {
        //   const double fit_pos = world.GetRandom().GetDouble(std::min(fitness_index.GetWeight(), 99999.9));
        //   const size_t parent_id = fitness_index.Index(fit_pos);
        //   const size_t offspring_id = world.DoBirth( world.GetGenomeAt(parent_id), parent_id ).index;
        //   if (world.IsSynchronous() == false) {
        //     fitness_index.Adjust(offspring_id, world.CalcFitnessID(offspring_id));
        //   }
        // }


    }

}

#endif
