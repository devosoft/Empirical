/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 *  @brief Data objects to be tracked per-taxon
 *
 *  Note that this file helps power the Python library phylotrackpy. The main consequence
 *  is that we should generally prefer emp_optional_throw to emp_assert in any circumstance
 *  where user input could trigger an error. emp_assert will trigger a segfault in Python
 *  (killing the whole interpreter), whereas emp_optional_throw will raise a Python exception.
 *

 */

#ifndef EMP_EVOLVE_SYSTEMATICS_DATA_HPP_INCLUDE
#define EMP_EVOLVE_SYSTEMATICS_DATA_HPP_INCLUDE

#include "SystematicsAnalysis.hpp"

namespace emp {


  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  class Systematics;

  /// The systematics manager allows an optional second template type that
  /// can store additional data about each taxon in the phylogeny. Here are
  /// some structs containing common pieces of additional data to track.
  /// Note: You are responsible for filling these in! Adding the template
  /// just gives you a place to store your data.

  namespace datastruct {
    struct no_data {
        using has_fitness_t = std::false_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;
        using has_num_offspring_ever_t = std::false_type;
        using has_total_orgs_t = std::false_type;
        using has_total_offspring_t = std::false_type;                
    }; /// The default - an empty struct

    struct fitness {
        using has_fitness_t = std::true_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;
        using has_num_offspring_ever_t = std::false_type;                
        using has_total_orgs_t = std::false_type;
        using has_total_offspring_t = std::false_type; 

        DataNode<double, data::Current, data::Range> fitness; /// This taxon's fitness (for assessing deleterious mutational steps)
        void RecordFitness(double fit) {
          fitness.Add(fit);
        }

        double GetFitness() const {
          return fitness.GetMean();
        }
    };

    struct cumulative_stats {
        using has_fitness_t = std::false_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;
        using has_num_offspring_ever_t = std::true_type;
        using has_total_orgs_t = std::true_type;
        using has_total_offspring_t = std::false_type;                

        int num_offspring_ever = 0;  // The total number of offspring this taxon has ever had
        int tot_orgs = 0;             // The total number of organisms that have ever lived associated with this taxon
    };

    struct cumulative_stats_all {
        using has_fitness_t = std::false_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;
        using has_num_offspring_ever_t = std::true_type;
        using has_total_orgs_t = std::true_type;
        using has_total_offspring_t = std::true_type;        

        int num_offspring_ever = 0;  // The total number of offspring this taxon has ever had
        int tot_orgs = 0;             // The total number of organisms that have ever lived associated with this taxon
        int total_offspring = 0;   ///<  How many total extant offspring taxa exist from this one (i.e. including indirect)        
    };


    /// Track information related to the mutational landscape
    /// Maps a string representing a type of mutation to a count representing
    /// the number of that type of mutation that occurred to bring about this taxon.
    template <typename PHEN_TYPE>
    struct mut_landscape_info {
      using phen_t = PHEN_TYPE;
      using has_phen_t = std::true_type;
      using has_mutations_t = std::true_type;
      using has_fitness_t = std::true_type;
      using has_num_offspring_ever_t = std::false_type;   
      using has_total_orgs_t = std::false_type;     
      using has_total_offspring_t = std::false_type;    

      // using has_phenotype_t = true;

      std::unordered_map<std::string, int> mut_counts = {}; /// The number of mutations of each type that occurred to make this taxon
      DataNode<double, data::Current, data::Range> fitness; /// This taxon's fitness (for assessing deleterious mutational steps)
      PHEN_TYPE phenotype; /// This taxon's phenotype (for assessing phenotypic change)

      /// @returns this taxon's phenotype
      const PHEN_TYPE & GetPhenotype() const {
        return phenotype;
      }

      /// @returns this taxon's fitness
      double GetFitness() const {
        return fitness.GetMean();
      }

      /// Adds mutations to the list of mutations that occurred to make this taxon
      /// @param muts can contain as many strings (types of mutation) as desired, each accompanied
      /// by a number indicating how many of that mutation occurred
      /// Example: {"point_mutation":2, "insertion":1}
      void RecordMutation(const std::unordered_map<std::string, int> & muts) {
        for (auto mut : muts) {
          if (Has(mut_counts, mut.first)) {
            mut_counts[mut.first] += mut.second;
          } else {
            mut_counts[mut.first] = mut.second;
          }
        }
      }

      /// Record the fitness of this taxon
      /// @param fit the fitness
      void RecordFitness(double fit) {
        fitness.Add(fit);
      }

      /// Record the phenotype of this taxon
      /// @param phen the phenotype
      void RecordPhenotype(PHEN_TYPE phen) {
        phenotype = phen;
      }

    };
  }
}

#endif