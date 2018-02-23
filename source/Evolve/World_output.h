/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 * This file contains functions for adding additional data files to Worlds.
**/

#ifndef EMP_EVO_WORLD_OUTPUT_H
#define EMP_EVO_WORLD_OUTPUT_H

#include "base/vector.h"
#include "World_file.h"     // Helper to determine when specific events should occur.
#include "SystematicsAnalysis.h"
#include "tools/string_utils.h"

namespace emp {

    template <typename WORLD_TYPE>
    World_file & AddPhylodiversityFile(WORLD_TYPE & world, const std::string & fpath="phylodiversity.csv"){
        auto & file = world.SetupFile(fpath);
        auto & distinctiveness_node = world.AddDataNode("evolutionary_distinctiveness");
        auto & pair_dist_node = world.AddDataNode("pairwise_distance");
        auto & phylo_div_node = world.AddDataNode("phylogenetic_diversity");

        world.OnUpdate([&distinctiveness_node, &pair_dist_node, &phylo_div_node, &world](size_t){
            distinctiveness_node.Reset();
            pair_dist_node.Reset();
            // Don't reset pd_node, because it's tracking summary statistics over time
            // (since there's only one PD for a given update)

            for (auto tax : world.GetSystematics().GetActive()) {
                distinctiveness_node.Add(world.GetSystematics().GetEvolutionaryDistinctiveness(tax, world.GetUpdate()));
            }

            emp::vector<int> mpd = world.GetSystematics().GetPairwiseDistances();
            for (int d : mpd) {
                pair_dist_node.Add(d);
            }
            phylo_div_node.Add(world.GetSystematics().GetPhylogeneticDiversity());
        });

        std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

        file.AddFun(get_update, "update", "Update");
        file.AddStats(distinctiveness_node, "evolutionary_distinctiveness", "evolutionary distinctiveness for a single update");
        file.AddStats(pair_dist_node, "pairwise_distance", "pairwise distance for a single update");
        file.AddStats(phylo_div_node, "phylogenetic_diversity", "phylogenetic diversity for entire run");
        file.AddCurrent(phylo_div_node, "current_phylogenetic_diversity", "current phylogenetic_diversity");
        file.PrintHeaderKeys();
        return file;
    }

    template <typename WORLD_TYPE>
    World_file & AddLineageMutationFile(WORLD_TYPE & world, const std::string & fpath="lineage_mutations.csv"){
        auto & file = world.SetupFile(fpath);
        auto & mut_count_node = world.AddDataNode("mut_count");
        auto & del_step_node = world.AddDataNode("deleterious_step");
        auto & phen_volatility_node = world.AddDataNode("phenotypic_volatility");
        auto & unique_phen_node = world.AddDataNode("unique_phenotypes");

        world.OnUpdate([&mut_count_node, &del_step_node, &phen_volatility_node, &unique_phen_node, &world](size_t){
            mut_count_node.Reset();
            del_step_node.Reset();
            phen_volatility_node.Reset();
            unique_phen_node.Reset();

            for (auto tax : world.GetSystematics().GetActive()) {
                mut_count_node.Add(CountMuts(tax));
                del_step_node.Add(CountDeleteriousSteps(tax));
                phen_volatility_node.Add(CountPhenotypeChanges(tax));
                unique_phen_node.Add(CountUniquePhenotypes(tax));
            }
        });


        std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

        file.AddFun(get_update, "update", "Update");
        file.AddStats(mut_count_node, "mutations_on_lineage", "counts of mutations along each lineage");
        file.AddStats(del_step_node, "deleterious_steps", "counts of deleterious steps along each lineage");
        file.AddStats(phen_volatility_node, "phenotypic_volatility", "counts of changes in phenotype along each lineage");
        file.AddStats(unique_phen_node, "unique_phenotypes", "counts of unique phenotypes along each lineage");
        file.PrintHeaderKeys();
        return file;
    }

    template <typename WORLD_TYPE>
    CollectionDataFile<std::unordered_set<Ptr<typename WORLD_TYPE::genotype_t>, typename Ptr<typename WORLD_TYPE::genotype_t>::hash_t>> AddMullerPlotFile(WORLD_TYPE & world, const std::string & fpath="muller_data.dat") {
        using taxon_t = typename WORLD_TYPE::genotype_t;
        using taxon_ptr_t = Ptr<typename WORLD_TYPE::genotype_t>;
        using container_t = std::unordered_set<taxon_ptr_t, typename taxon_ptr_t::hash_t>;

        CollectionDataFile<container_t> file(fpath);
        file.SetUpdateContainerFun([&world](){return world.GetSystematics().GetActive();});

        std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};
        std::function<int(const taxon_ptr_t)> get_tax_id = [&world](const taxon_ptr_t t){
            return t->GetID();
        };
        std::function<int(const taxon_ptr_t)> get_num_orgs = [&world](const taxon_ptr_t t){
            return t->GetNumOrgs();
        };
        std::function<std::string(const taxon_ptr_t)> get_phenotype = [&world](const taxon_ptr_t t){
            return to_string(t->GetData().GetPhenotype());
        };

        file.AddFun(get_update, "update", "Update");
        file.AddCollectionFun(get_tax_id, "tax_id", "The id of the taxon for this row");
        file.AddCollectionFun(get_num_orgs, "num_orgs", "The number of orgs of this taxon on this update");
        file.AddCollectionFun(get_phenotype, "phenotype", "This taxon's phenotype");
        file.PrintHeaderKeys();
        return file;
    }

};

#endif
