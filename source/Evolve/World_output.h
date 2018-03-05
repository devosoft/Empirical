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
#include "data/DataFile.h"     // Helper to determine when specific events should occur.
#include "SystematicsAnalysis.h"
#include "tools/string_utils.h"

namespace emp {

    template <typename WORLD_TYPE>
    DataFile & AddPhylodiversityFile(WORLD_TYPE & world, const std::string & fpath="phylodiversity.csv"){
        auto & file = world.SetupFile(fpath);
        Ptr<DataMonitor<double>> distinctiveness_node = world.AddDataNode("evolutionary_distinctiveness");
        Ptr<DataMonitor<double>> pair_dist_node = world.AddDataNode("pairwise_distance");
        Ptr<DataMonitor<double>> phylo_div_node = world.AddDataNode("phylogenetic_diversity");

        world.OnUpdate([distinctiveness_node, pair_dist_node, phylo_div_node, &world](size_t ud) mutable -> void {
            distinctiveness_node->Reset();
            pair_dist_node->Reset();
            // Don't reset pd_node, because it's tracking summary statistics over time
            // (since there's only one PD for a given update)

            for (auto tax : world.GetSystematics().GetActive()) {
                distinctiveness_node->Add(world.GetSystematics().GetEvolutionaryDistinctiveness(tax, world.GetUpdate()));
            }

            emp::vector<int> mpd = world.GetSystematics().GetPairwiseDistances();
            for (int d : mpd) {
                pair_dist_node->Add(d);
            }
            phylo_div_node->Add(world.GetSystematics().GetPhylogeneticDiversity());
        });

        std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

        file.AddFun(get_update, "update", "Update");
        file.AddStats(*distinctiveness_node, "evolutionary_distinctiveness", "evolutionary distinctiveness for a single update");
        file.AddStats(*pair_dist_node, "pairwise_distance", "pairwise distance for a single update");
        file.AddStats(*phylo_div_node, "phylogenetic_diversity", "phylogenetic diversity for entire run");
        file.AddCurrent(*phylo_div_node, "current_phylogenetic_diversity", "current phylogenetic_diversity");
        file.PrintHeaderKeys();
        return file;
    }

    template <typename WORLD_TYPE>
    DataFile & AddLineageMutationFile(WORLD_TYPE & world, const std::string & fpath="lineage_mutations.csv", emp::vector<std::string> mut_types = {"substitution"}){
        auto & file = world.SetupFile(fpath);
        emp::vector<Ptr<DataMonitor<double>>> mut_count_nodes;
        for (size_t i = 0; i < mut_types.size(); i++) {
            mut_count_nodes.push_back(world.AddDataNode(mut_types[i]+"_mut_count"));
        }

        Ptr<DataMonitor<double>> del_step_node = world.AddDataNode("deleterious_step");
        Ptr<DataMonitor<double>> phen_volatility_node = world.AddDataNode("phenotypic_volatility");
        Ptr<DataMonitor<double>> unique_phen_node = world.AddDataNode("unique_phenotypes");

        world.OnUpdate([mut_count_nodes, mut_types, del_step_node, phen_volatility_node, unique_phen_node, &world](size_t ud) mutable -> void {
            for (Ptr<DataMonitor<double>> node : mut_count_nodes) {
                node->Reset();
            }
            del_step_node->Reset();
            phen_volatility_node->Reset();
            unique_phen_node->Reset();

            for (auto tax : world.GetSystematics().GetActive()) {
                for (size_t i = 0; i < mut_types.size(); i++) {
                    mut_count_nodes[i]->Add(CountMuts(tax, mut_types[i]));
                }

                del_step_node->Add(CountDeleteriousSteps(tax));
                phen_volatility_node->Add(CountPhenotypeChanges(tax));
                unique_phen_node->Add(CountUniquePhenotypes(tax));
            }
        });


        std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};

        file.AddFun(get_update, "update", "Update");
        for (size_t i = 0; i < mut_types.size(); i++) {
            file.AddStats(*(mut_count_nodes[i]), mut_types[i] + "_mutations_on_lineage", "counts of" + mut_types[i] + "mutations along each lineage");
        }

        file.AddStats(*del_step_node, "deleterious_steps", "counts of deleterious steps along each lineage");
        file.AddStats(*phen_volatility_node, "phenotypic_volatility", "counts of changes in phenotype along each lineage");
        file.AddStats(*unique_phen_node, "unique_phenotypes", "counts of unique phenotypes along each lineage");
        file.PrintHeaderKeys();
        return file;
    }

    template <typename WORLD_TYPE>
    auto
    AddMullerPlotFile(WORLD_TYPE & world, const std::string & fpath="muller_data.dat") -> ContainerDataFile<decltype(world.GetSystematics().GetActivePtr())> & {

        using taxon_ptr_t = Ptr<typename WORLD_TYPE::genotype_t>;
        using container_t = decltype(world.GetSystematics().GetActivePtr());
        
        Ptr<ContainerDataFile<container_t>> file;
        file.New(fpath);

        auto & world_file = static_cast<ContainerDataFile<container_t>&>(world.AddDataFile(file));

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
        std::function<std::string(const taxon_ptr_t)> get_parent = [&world](const taxon_ptr_t t){
            if (t->GetParent()) {
                return to_string(t->GetParent()->GetID());
            } else {
                return to_string("-1");
            }
        };

        world_file.SetUpdateContainerFun([&world](){return world.GetSystematics().GetActivePtr();});

        world_file.AddFun(get_update, "update", "Update");
        world_file.AddContainerFun(get_tax_id, "tax_id", "The id of the taxon for this row");
        world_file.AddContainerFun(get_num_orgs, "num_orgs", "The number of orgs of this taxon on this update");
        world_file.AddContainerFun(get_phenotype, "phenotype", "This taxon's phenotype");
        world_file.AddContainerFun(get_parent, "parent_id", "This taxon's parent's id");
        world_file.PrintHeaderKeys();
        
        return world_file;
    }

};

#endif
