/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file World_output.hpp
 *  @brief TODO.
 *
 * This file contains functions for adding additional data files to Worlds.
 */

#ifndef EMP_EVOLVE_WORLD_OUTPUT_HPP_INCLUDE
#define EMP_EVOLVE_WORLD_OUTPUT_HPP_INCLUDE

#include "../base/vector.hpp"
#include "../data/DataFile.hpp"   // Helper to determine when specific events should occur.
#include "../tools/string_utils.hpp"

#include "SystematicsAnalysis.hpp"

namespace emp {

  template <typename WORLD_TYPE, typename OEE_TYPE>
  DataFile & AddOEEFile(WORLD_TYPE & world, OEE_TYPE & oee_tracker, const std::string fpath = "oee_data.csv") {
    auto & file = world.SetupFile(fpath);
    std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

    file.AddFun(get_update, "update", "Update");
    file.AddCurrent(*oee_tracker.GetDataNode("change"), "change", "change potential");
    file.AddCurrent(*oee_tracker.GetDataNode("novelty"), "novelty", "novelty potential");
    file.AddCurrent(*oee_tracker.GetDataNode("diversity"), "ecology", "ecology potential");
    file.AddCurrent(*oee_tracker.GetDataNode("complexity"), "complexity", "complexity potential");
    file.PrintHeaderKeys();
    return file;
  }

  template <typename WORLD_TYPE>
  DataFile & AddPhylodiversityFile(WORLD_TYPE & world, int systematics_id=0, const std::string & fpath="phylodiversity.csv"){
    auto & file = world.SetupFile(fpath);
    auto sys = world.GetSystematics(systematics_id);

    sys->AddEvolutionaryDistinctivenessDataNode();
    sys->AddPairwiseDistanceDataNode();
    sys->AddPhylogeneticDiversityDataNode();

    std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

    file.AddFun(get_update, "update", "Update");
    file.AddStats(*sys->GetDataNode("evolutionary_distinctiveness") , "evolutionary_distinctiveness", "evolutionary distinctiveness for a single update", true, true);
    file.AddStats(*sys->GetDataNode("pairwise_distance"), "pairwise_distance", "pairwise distance for a single update", true, true);
    file.AddCurrent(*sys->GetDataNode("phylogenetic_diversity"), "current_phylogenetic_diversity", "current phylogenetic_diversity", true, true);
    file.PrintHeaderKeys();
    return file;
  }

  template <typename WORLD_TYPE>
  DataFile & AddLineageMutationFile(WORLD_TYPE & world, int systematics_id=0, const std::string & fpath="lineage_mutations.csv", emp::vector<std::string> mut_types = {"substitution"}){
    auto & file = world.SetupFile(fpath);
    auto sys = world.GetSystematics(systematics_id);

    for (size_t i = 0; i < mut_types.size(); i++) {
      sys->AddMutationCountDataNode(mut_types[i]+"_mut_count", mut_types[i]);
    }

    auto node = sys->AddDeleteriousStepDataNode();
    sys->AddVolatilityDataNode();
    sys->AddUniqueTaxaDataNode();

    std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

    file.AddFun(get_update, "update", "Update");
    for (size_t i = 0; i < mut_types.size(); i++) {
      file.AddStats(*sys->GetDataNode(mut_types[i]+"_mut_count"), mut_types[i] + "_mutations_on_lineage", "counts of" + mut_types[i] + "mutations along each lineage", true, true);
    }

    file.AddStats(*sys->GetDataNode("deleterious_steps"), "deleterious_steps", "counts of deleterious steps along each lineage", true, true);
    file.AddStats(*sys->GetDataNode("volatility"), "taxon_volatility", "counts of changes in taxon along each lineage", true, true);
    file.AddStats(*sys->GetDataNode("unique_taxa"), "unique_taxa", "counts of unique taxa along each lineage", true, true);
    file.PrintHeaderKeys();
    return file;
  }

};

#endif // #ifndef EMP_EVOLVE_WORLD_OUTPUT_HPP_INCLUDE
