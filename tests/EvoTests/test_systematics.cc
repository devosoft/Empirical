#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../third-party/Catch/single_include/catch.hpp"

#include "Evolve/Systematics.h"
#include "Evolve/SystematicsAnalysis.h"
#include "Evolve/World.h"
#include "base/vector.h"
#include <iostream>
#include "hardware/AvidaGP.h"
#include "Evolve/World_output.h"

TEST_CASE("Test Systematics", "[evo]")
{
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  std::cout << "\nAddOrg 25 (id1, no parent)\n";
  auto id1 = sys.AddOrg(25, nullptr, 0);
  std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  auto id2 = sys.AddOrg(-10, id1, 6);
  std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  auto id3 = sys.AddOrg(26, id1, 10);
  std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  auto id4 = sys.AddOrg(27, id2, 25);
  std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  auto id5 = sys.AddOrg(28, id2, 32);
  std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  auto id6 = sys.AddOrg(29, id5, 39);
  std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  auto id7 = sys.AddOrg(30, id1, 6);


  std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);

  double mpd = sys.GetMeanPairwiseDistance();
  std::cout << "MPD: " << mpd <<std::endl;
  REQUIRE(mpd == Approx(2.8));

  std::cout << "\nAddOrg 31 (id8; parent id7)\n";
  auto id8 = sys.AddOrg(31, id7, 11);
  std::cout << "\nAddOrg 32 (id9; parent id8)\n";
  auto id9 = sys.AddOrg(32, id8, 19);


  REQUIRE(sys.GetEvolutionaryDistinctiveness(id3, 10) == 10);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id4, 25) == 21);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id5, 32) == 15);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id6, 39) == 22);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id6, 45) == 28);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id9, 19) == 12.5);

  std::cout << "\nAddOrg 33 (id10; parent id8)\n";
  auto id10 = sys.AddOrg(33, id8, 19);

  sys.RemoveOrg(id7);
  sys.RemoveOrg(id8);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id9, 19) == 13.5);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id10, 19) == 13.5);

  sys.RemoveOrg(id10);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id9, 19) == 19);


  std::cout << "\nAddOrg 34 (id11; parent id9)\n";
  auto id11 = sys.AddOrg(34, id9, 22);
  std::cout << "\nAddOrg 35 (id12; parent id10)\n";
  auto id12 = sys.AddOrg(35, id11, 23);

  sys.RemoveOrg(id9);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id11, 26) == 13);
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id12, 26) == 15);

  std::cout << "\nAddOrg 36 (id13; parent id12)\n";
  auto id13 = sys.AddOrg(36, id12, 27);
  std::cout << "\nAddOrg 37 (id14; parent id13)\n";
  auto id14 = sys.AddOrg(37, id13, 30);

  sys.RemoveOrg(id13);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id14, 33) == Approx(17.833333));

  std::cout << "\nAddOrg 38 (id15; parent id14)\n";
  auto id15 = sys.AddOrg(38, id14, 33);

  sys.RemoveOrg(id14);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id15, 33) == Approx(17.833333));

  std::cout << "\nAddOrg 39 (id16; parent id11)\n";
  auto id16 = sys.AddOrg(39, id11, 35);
  std::cout << "\nAddOrg 40 (id17; parent id11)\n";
  auto id17 = sys.AddOrg(40, id11, 35);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id16, 35) == Approx(17.4));
  REQUIRE(sys.GetEvolutionaryDistinctiveness(id17, 35) == Approx(17.4));

  std::cout << "\nAddOrg 41 (id18; parent id17)\n";
  auto id18 = sys.AddOrg(41, id17, 36);

  REQUIRE(sys.GetEvolutionaryDistinctiveness(id18, 37) == Approx(12.1666667));

  REQUIRE(sys.GetTaxonDistinctiveness(id18) == Approx(1.0/6.0));
  REQUIRE(sys.GetBranchesToRoot(id18) == 1);
  REQUIRE(sys.GetDistanceToRoot(id18) == 6);

  std::cout << "\nAddOrg 42 (id19; parent id17)\n";
  auto id19 = sys.AddOrg(42, id17, 37);
  REQUIRE(sys.GetBranchesToRoot(id19) == 2);
  REQUIRE(sys.GetDistanceToRoot(id19) == 6);
  REQUIRE(sys.GetTaxonDistinctiveness(id19) == Approx(1.0/6.0));

  REQUIRE(sys.GetTaxonDistinctiveness(id15) == Approx(1.0/8.0));
  REQUIRE(sys.GetBranchesToRoot(id15) == 1);
  REQUIRE(sys.GetDistanceToRoot(id15) == 8);
  REQUIRE(sys.GetPhylogeneticDiversity() == 17);
  REQUIRE(sys.GetAveDepth() == Approx(4.272727));

  std::cout << "id1 = " << id1 << std::endl;
  std::cout << "id2 = " << id2 << std::endl;
  std::cout << "id3 = " << id3 << std::endl;
  std::cout << "id4 = " << id4 << std::endl;

  std::stringstream result;

  sys.PrintLineage(id4, result);
  sys.PrintStatus();

  REQUIRE(result.str() == "Lineage:\n27\n-10\n25\n");

  CHECK(sys.GetStoreActive() == 1);
  CHECK(sys.GetStoreAncestors() == 1);
  CHECK(sys.GetStoreOutside() == 1);
  CHECK(sys.GetArchive() == 1);
  CHECK(sys.GetTrackSynchronous() == 0);
  CHECK(sys.GetNextID() == 19);
  CHECK(sys.GetNumActive() == 11);
  CHECK(sys.GetNumAncestors() == 7);
  CHECK(sys.GetNumOutside() == 1);

  auto ancestors = sys.GetAncestors();
  emp::vector<emp::Ptr<emp::Taxon<int>>> ancestor_vec(ancestors.begin(), ancestors.end());
  emp::Sort(ancestor_vec, [](emp::Ptr<emp::Taxon<int>> & a, emp::Ptr<emp::Taxon<int>> & b){
    return a->GetID() < b->GetID();
  });

  CHECK(ancestor_vec[0]->GetID() == 1);
  CHECK(ancestor_vec[0]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[0]->GetNumOff() == 3);
  CHECK(ancestor_vec[0]->GetParent() == nullptr);

  CHECK(ancestor_vec[1]->GetID() == 2);
  CHECK(ancestor_vec[1]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[1]->GetNumOff() == 2);
  CHECK(ancestor_vec[1]->GetParent()->GetID() == 1);

  CHECK(ancestor_vec[2]->GetID() == 7);
  CHECK(ancestor_vec[2]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[2]->GetNumOff() == 1);
  CHECK(ancestor_vec[2]->GetParent()->GetID() == 1);

  CHECK(ancestor_vec[3]->GetID() == 8);
  CHECK(ancestor_vec[3]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[3]->GetNumOff() == 1);
  CHECK(ancestor_vec[3]->GetParent()->GetID() == 7);

  CHECK(ancestor_vec[4]->GetID() == 9);
  CHECK(ancestor_vec[4]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[4]->GetNumOff() == 1);
  CHECK(ancestor_vec[4]->GetParent()->GetID() == 8);

  CHECK(ancestor_vec[5]->GetID() == 13);
  CHECK(ancestor_vec[5]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[5]->GetNumOff() == 1);
  CHECK(ancestor_vec[5]->GetParent()->GetID() == 12);

  CHECK(ancestor_vec[6]->GetID() == 14);
  CHECK(ancestor_vec[6]->GetNumOrgs() == 0);
  CHECK(ancestor_vec[6]->GetNumOff() == 1);
  CHECK(ancestor_vec[6]->GetParent()->GetID() == 13);

  auto outside_taxon = *(sys.GetOutside().begin());
  CHECK(outside_taxon->GetID() == 10);
  CHECK(outside_taxon->GetNumOrgs() == 0);
  CHECK(outside_taxon->GetNumOff() == 0);
  CHECK(outside_taxon->GetParent()->GetID() == 8);

  auto active = sys.GetActive();
  emp::vector<emp::Ptr<emp::Taxon<int>>> active_vec(active.begin(), active.end());
  emp::Sort(active_vec, [](emp::Ptr<emp::Taxon<int>> & a, emp::Ptr<emp::Taxon<int>> & b){
    return a->GetID() < b->GetID();
  });

  CHECK(active_vec[0]->GetID() == 3);
  CHECK(active_vec[0]->GetNumOrgs() == 1);
  CHECK(active_vec[0]->GetNumOff() == 0);
  CHECK(active_vec[0]->GetParent()->GetID() == 1);

  CHECK(active_vec[1]->GetID() == 4);
  CHECK(active_vec[1]->GetNumOrgs() == 1);
  CHECK(active_vec[1]->GetNumOff() == 0);
  CHECK(active_vec[1]->GetParent()->GetID() == 2);

  CHECK(active_vec[2]->GetID() == 5);
  CHECK(active_vec[2]->GetNumOrgs() == 1);
  CHECK(active_vec[2]->GetNumOff() == 1);
  CHECK(active_vec[2]->GetParent()->GetID() == 2);

  CHECK(active_vec[3]->GetID() == 6);
  CHECK(active_vec[3]->GetNumOrgs() == 1);
  CHECK(active_vec[3]->GetNumOff() == 0);
  CHECK(active_vec[3]->GetParent()->GetID() == 5);

  CHECK(active_vec[4]->GetID() == 11);
  CHECK(active_vec[4]->GetNumOrgs() == 1);
  CHECK(active_vec[4]->GetNumOff() == 3);
  CHECK(active_vec[4]->GetParent()->GetID() == 9);

  CHECK(active_vec[5]->GetID() == 12);
  CHECK(active_vec[5]->GetNumOrgs() == 1);
  CHECK(active_vec[5]->GetNumOff() == 1);
  CHECK(active_vec[5]->GetParent()->GetID() == 11);

  CHECK(active_vec[6]->GetID() == 15);
  CHECK(active_vec[6]->GetNumOrgs() == 1);
  CHECK(active_vec[6]->GetNumOff() == 0);
  CHECK(active_vec[6]->GetParent()->GetID() == 14);

  CHECK(active_vec[7]->GetID() == 16);
  CHECK(active_vec[7]->GetNumOrgs() == 1);
  CHECK(active_vec[7]->GetNumOff() == 0);
  CHECK(active_vec[7]->GetParent()->GetID() == 11);

  CHECK(active_vec[8]->GetID() == 17);
  CHECK(active_vec[8]->GetNumOrgs() == 1);
  CHECK(active_vec[8]->GetNumOff() == 2);
  CHECK(active_vec[8]->GetParent()->GetID() == 11);

  CHECK(active_vec[9]->GetID() == 18);
  CHECK(active_vec[9]->GetNumOrgs() == 1);
  CHECK(active_vec[9]->GetNumOff() == 0);
  CHECK(active_vec[9]->GetParent()->GetID() == 17);

  CHECK(active_vec[10]->GetID() == 19);
  CHECK(active_vec[10]->GetNumOrgs() == 1);
  CHECK(active_vec[10]->GetNumOff() == 0);
  CHECK(active_vec[10]->GetParent()->GetID() == 17);

}

TEST_CASE("Test not tracking ancestors", "[evo]")
{
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, false, false, false);

  std::cout << "\nAddOrg 25 (id1, no parent)\n";
  auto id1 = sys.AddOrg(25, nullptr, 0);
  std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  auto id2 = sys.AddOrg(-10, id1, 6);
  std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  auto id3 = sys.AddOrg(26, id1, 10);
  std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  auto id4 = sys.AddOrg(27, id2, 25);
  std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  auto id5 = sys.AddOrg(28, id2, 32);
  std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  auto id6 = sys.AddOrg(29, id5, 39);
  std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  auto id7 = sys.AddOrg(30, id1, 6);
  

  std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);

  double mpd = sys.GetMeanPairwiseDistance();

  std::cout << "\nAddOrg 31 (id8; parent id7)\n";
  auto id8 = sys.AddOrg(31, id7, 11);
  std::cout << "\nAddOrg 32 (id9; parent id8)\n";
  auto id9 = sys.AddOrg(32, id8, 19);

  std::cout << "\nAddOrg 33 (id10; parent id8)\n";
  auto id10 = sys.AddOrg(33, id8, 19);

  sys.RemoveOrg(id7);
  sys.RemoveOrg(id8);

  sys.RemoveOrg(id10);


  std::cout << "\nAddOrg 34 (id11; parent id9)\n";
  auto id11 = sys.AddOrg(34, id9, 22);
  std::cout << "\nAddOrg 35 (id12; parent id10)\n";
  auto id12 = sys.AddOrg(35, id11, 23);

  sys.RemoveOrg(id9);

  std::cout << "\nAddOrg 36 (id13; parent id12)\n";
  auto id13 = sys.AddOrg(36, id12, 27);
  std::cout << "\nAddOrg 37 (id14; parent id13)\n";
  auto id14 = sys.AddOrg(37, id13, 30);

  sys.RemoveOrg(id13);

  std::cout << "\nAddOrg 38 (id15; parent id14)\n";
  auto id15 = sys.AddOrg(38, id14, 33);

  sys.RemoveOrg(id14);

  std::cout << "\nAddOrg 39 (id16; parent id11)\n";
  auto id16 = sys.AddOrg(39, id11, 35);
  std::cout << "\nAddOrg 40 (id17; parent id11)\n";
  auto id17 = sys.AddOrg(40, id11, 35);

  std::cout << "\nAddOrg 41 (id18; parent id17)\n";
  auto id18 = sys.AddOrg(41, id17, 36);

  std::cout << "\nAddOrg 42 (id19; parent id17)\n";
  auto id19 = sys.AddOrg(42, id17, 37);
  REQUIRE(id17->GetTotalOffspring() > 0);

  std::cout << "id3 = " << id3 << std::endl;
  std::cout << "id4 = " << id4 << std::endl;

  std::stringstream result;

  sys.PrintLineage(id4, result);
  sys.PrintStatus();
  CHECK(result.str() == "Lineage:\n27\n");

  CHECK(sys.GetStoreActive() == 1);
  CHECK(sys.GetStoreAncestors() == 0);
  CHECK(sys.GetStoreOutside() == 0);
  CHECK(sys.GetArchive() == 0);
  CHECK(sys.GetTrackSynchronous() == 0);
  CHECK(sys.GetNextID() == 19);
  CHECK(sys.GetNumActive() == 11);
  CHECK(sys.GetNumAncestors() == 0);
  CHECK(sys.GetNumOutside() == 0);

  auto active = sys.GetActive();
  emp::vector<emp::Ptr<emp::Taxon<int>>> active_vec(active.begin(), active.end());
  emp::Sort(active_vec, [](emp::Ptr<emp::Taxon<int>> & a, emp::Ptr<emp::Taxon<int>> & b){
    return a->GetID() < b->GetID();
  });

  CHECK(active_vec[0]->GetID() == 3);
  CHECK(active_vec[0]->GetNumOrgs() == 1);
  CHECK(active_vec[0]->GetNumOff() == 0);
  CHECK(active_vec[0]->GetParent() == nullptr);

  CHECK(active_vec[1]->GetID() == 4);
  CHECK(active_vec[1]->GetNumOrgs() == 1);
  CHECK(active_vec[1]->GetNumOff() == 0);
  CHECK(active_vec[1]->GetParent() == nullptr);

  CHECK(active_vec[2]->GetID() == 5);
  CHECK(active_vec[2]->GetNumOrgs() == 1);
  CHECK(active_vec[2]->GetNumOff() == 1);
  CHECK(active_vec[2]->GetParent() == nullptr);

  CHECK(active_vec[3]->GetID() == 6);
  CHECK(active_vec[3]->GetNumOrgs() == 1);
  CHECK(active_vec[3]->GetNumOff() == 0);
  CHECK(active_vec[3]->GetParent()->GetID() == 5);

  CHECK(active_vec[4]->GetID() == 11);
  CHECK(active_vec[4]->GetNumOrgs() == 1);
  CHECK(active_vec[4]->GetNumOff() == 3);
  CHECK(active_vec[4]->GetParent() == nullptr);

  CHECK(active_vec[5]->GetID() == 12);
  CHECK(active_vec[5]->GetNumOrgs() == 1);
  CHECK(active_vec[5]->GetNumOff() == 1);
  CHECK(active_vec[5]->GetParent()->GetID() == 11);

  CHECK(active_vec[6]->GetID() == 15);
  CHECK(active_vec[6]->GetNumOrgs() == 1);
  CHECK(active_vec[6]->GetNumOff() == 0);
  CHECK(active_vec[6]->GetParent() == nullptr);

  CHECK(active_vec[7]->GetID() == 16);
  CHECK(active_vec[7]->GetNumOrgs() == 1);
  CHECK(active_vec[7]->GetNumOff() == 0);
  CHECK(active_vec[7]->GetParent()->GetID() == 11);

  CHECK(active_vec[8]->GetID() == 17);
  CHECK(active_vec[8]->GetNumOrgs() == 1);
  CHECK(active_vec[8]->GetNumOff() == 2);
  CHECK(active_vec[8]->GetParent()->GetID() == 11);

  CHECK(active_vec[9]->GetID() == 18);
  CHECK(active_vec[9]->GetNumOrgs() == 1);
  CHECK(active_vec[9]->GetNumOff() == 0);
  CHECK(active_vec[9]->GetParent()->GetID() == 17);

  CHECK(active_vec[10]->GetID() == 19);
  CHECK(active_vec[10]->GetNumOrgs() == 1);
  CHECK(active_vec[10]->GetNumOff() == 0);
  CHECK(active_vec[10]->GetParent()->GetID() == 17);

}


TEST_CASE("Pointer to systematics", "[evo]") {
  emp::Ptr<emp::Systematics<int, int>> sys;
  sys.New([](const int & i){return i;}, true, true, true);
  sys.Delete();
}

TEST_CASE("Test Data Struct", "[evo]")
{

  emp::Ptr<emp::Systematics<int, int, emp::datastruct::mut_landscape_info<int> >> sys;
  sys.New([](const int & i){return i;}, true, true, true, false);
  auto id1 = sys->AddOrg(1, nullptr);
  id1->GetData().fitness.Add(2);
  id1->GetData().phenotype = 6;

  auto id2 = sys->AddOrg(2, id1);
  id2->GetData().mut_counts["substitution"] = 2;
  id2->GetData().fitness.Add(1);
  id2->GetData().phenotype = 6;
  CHECK(id2->GetData().mut_counts["substitution"] == 2);

  auto id3 = sys->AddOrg(3, id1);
  id3->GetData().mut_counts["substitution"] = 5;
  id3->GetData().fitness.Add(0);
  id3->GetData().phenotype = 6;

  auto id4 = sys->AddOrg(4, id2);
  id4->GetData().mut_counts["substitution"] = 1;
  id4->GetData().fitness.Add(3);
  id4->GetData().phenotype = 3;

  auto id5 = sys->AddOrg(5, id4);
  id5->GetData().mut_counts["substitution"] = 1;
  id5->GetData().fitness.Add(2);
  id5->GetData().phenotype = 6;


  CHECK(CountMuts(id4) == 3);
  CHECK(CountDeleteriousSteps(id4) == 1);
  CHECK(CountPhenotypeChanges(id4) == 1);
  CHECK(CountUniquePhenotypes(id4) == 2);

  CHECK(CountMuts(id3) == 5);
  CHECK(CountDeleteriousSteps(id3) == 1);
  CHECK(CountPhenotypeChanges(id3) == 0);
  CHECK(CountUniquePhenotypes(id3) == 1);

  CHECK(CountMuts(id5) == 4);
  CHECK(CountDeleteriousSteps(id5) == 2);
  CHECK(CountPhenotypeChanges(id5) == 2);
  CHECK(CountUniquePhenotypes(id5) == 2);

  sys.Delete();

}


TEST_CASE("World systematics integration", "[evo]") {

  // std::function<void(emp::Ptr<emp::Taxon<emp::vector<int>, emp::datastruct::mut_landscape_info<int>>>)> setup_phenotype = [](emp::Ptr<emp::Taxon<emp::vector<int>, emp::datastruct::mut_landscape_info<int>>> tax){
  //   tax->GetData().phenotype = emp::Sum(tax->GetInfo());
  // };

  using systematics_t = emp::Systematics<
      emp::vector<int>,
      emp::vector<int>,
      emp::datastruct::mut_landscape_info< int >
    >;

  emp::World<emp::vector<int>> world;
  emp::Ptr<systematics_t> sys;
  sys.New([](const emp::vector<int> & v){return v;}, true, true, true);
  world.AddSystematics(sys);

  world.SetMutFun([](emp::vector<int> & org, emp::Random & r){return 0;});

  // world.GetSystematics().OnNew(setup_phenotype);
  world.InjectAt(emp::vector<int>({1,2,3}), 0);

  sys->GetTaxonAt(0)->GetData().RecordPhenotype(6);
  sys->GetTaxonAt(0)->GetData().RecordFitness(2);

  REQUIRE(sys->GetTaxonAt(0)->GetData().phenotype == 6);

  std::unordered_map<std::string, int> mut_counts;
  mut_counts["substitution"] = 3;

  emp::vector<int> new_org({4,2,3});
  auto old_taxon = sys->GetTaxonAt(0);
  world.DoBirth(new_org,0);

  REQUIRE(old_taxon->GetNumOrgs() == 0);
  REQUIRE(old_taxon->GetNumOff() == 1);
  REQUIRE(sys->GetTaxonAt(0)->GetParent()->GetData().phenotype == 6);
  REQUIRE((*sys->GetActive().begin())->GetNumOrgs() == 1);

}

template <typename WORLD_TYPE>
emp::DataFile AddDominantFile(WORLD_TYPE & world){
  using mut_count_t [[maybe_unused]] = std::unordered_map<std::string, int>;
  using data_t = emp::datastruct::mut_landscape_info<emp::vector<double>>;
  using org_t = emp::AvidaGP;
  using systematics_t = emp::Systematics<org_t, org_t, data_t>;


    auto & file = world.SetupFile("dominant.csv");

    std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};
    std::function<int(void)> dom_mut_count = [&world](){
      return CountMuts(dynamic_cast<emp::Ptr<systematics_t>>(world.GetSystematics(0))->GetTaxonAt(0));
    };
    std::function<int(void)> dom_del_step = [&world](){
      return CountDeleteriousSteps(dynamic_cast<emp::Ptr<systematics_t>>(world.GetSystematics(0))->GetTaxonAt(0));
    };
    std::function<size_t(void)> dom_phen_vol = [&world](){
      return CountPhenotypeChanges(dynamic_cast<emp::Ptr<systematics_t>>(world.GetSystematics(0))->GetTaxonAt(0));
    };
    std::function<size_t(void)> dom_unique_phen = [&world](){
      return CountUniquePhenotypes(dynamic_cast<emp::Ptr<systematics_t>>(world.GetSystematics(0))->GetTaxonAt(0));
    };


    file.AddFun(get_update, "update", "Update");
    file.AddFun(dom_mut_count, "dominant_mutation_count", "sum of mutations along dominant organism's lineage");
    file.AddFun(dom_del_step, "dominant_deleterious_steps", "count of deleterious steps along dominant organism's lineage");
    file.AddFun(dom_phen_vol, "dominant_phenotypic_volatility", "count of changes in phenotype along dominant organism's lineage");
    file.AddFun(dom_unique_phen, "dominant_unique_phenotypes", "count of unique phenotypes along dominant organism's lineage");
    file.PrintHeaderKeys();
    return file;
}

TEST_CASE("Run world", "[evo]") {
  using mut_count_t = std::unordered_map<std::string, int>;
  using data_t = emp::datastruct::mut_landscape_info<emp::vector<double>>;
  using org_t = emp::AvidaGP;
  using gene_systematics_t = emp::Systematics<org_t, org_t::genome_t, data_t>;
  using phen_systematics_t = emp::Systematics<org_t, emp::vector<double>, data_t>;

  emp::Random random(1);
  emp::World<org_t> world(random, "AvidaWorld");
  world.SetPopStruct_Mixed(true);


  std::function<emp::AvidaGP::genome_t(const emp::AvidaGP &)> gene_fun =
    [](const emp::AvidaGP & org) {
      return org.GetGenome();
    };

  std::function<emp::vector<double>(const emp::AvidaGP &)> phen_fun =
    [](const emp::AvidaGP & org) {
      emp::vector<double> phen;
      emp::AvidaGP org2 = org;
      for (int i = 0; i < 16; i++) {
        org2.ResetHardware();
        org2.Process(20);
        phen.push_back(org2.GetOutput(i));
      }
      return phen;
    };

  mut_count_t last_mutation;
  emp::Ptr<gene_systematics_t> gene_sys;
  emp::Ptr<phen_systematics_t> phen_sys;
  gene_sys.New(gene_fun, true,true,true);
  phen_sys.New(phen_fun, true,true,true);
  world.AddSystematics(gene_sys);
  world.AddSystematics(phen_sys);

  emp::Signal<void(mut_count_t)> on_mutate_sig;    ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, double)> record_fit_sig;    ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, emp::vector<double>)> record_phen_sig;    ///< Trigger signal before organism gives birth.

  on_mutate_sig.AddAction([&last_mutation](mut_count_t muts){last_mutation = muts;});

  record_fit_sig.AddAction([&world](size_t pos, double fit){
    world.GetSystematics(0).Cast<gene_systematics_t>()->GetTaxonAt(pos)->GetData().RecordFitness(fit);
    world.GetSystematics(1).Cast<phen_systematics_t>()->GetTaxonAt(pos)->GetData().RecordFitness(fit);
  });

  record_phen_sig.AddAction([&world](size_t pos, emp::vector<double> phen){
    world.GetSystematics(0).Cast<gene_systematics_t>()->GetTaxonAt(pos)->GetData().RecordPhenotype(phen);
    world.GetSystematics(1).Cast<phen_systematics_t>()->GetTaxonAt(pos)->GetData().RecordPhenotype(phen);
  });

  // world.OnOrgPlacement([&last_mutation, &world](size_t pos){
  //   world.GetSystematics(0).Cast<systematics_t>()->GetTaxonAt(pos)->GetData().RecordMutation(last_mutation);
  // });

  world.SetupSystematicsFile().SetTimingRepeat(1);
  world.SetupFitnessFile().SetTimingRepeat(1);
  world.SetupPopulationFile().SetTimingRepeat(1);
  emp::AddPhylodiversityFile(world, 0, "genotype_phylodiversity.csv").SetTimingRepeat(1);
  emp::AddPhylodiversityFile(world, 1, "phenotype_phylodiversity.csv").SetTimingRepeat(1);
  emp::AddLineageMutationFile(world).SetTimingRepeat(1);
  // AddDominantFile(world).SetTimingRepeat(1);
  // emp::AddMullerPlotFile(world).SetTimingOnce(1);


  // Setup the mutation function.
  world.SetMutFun( [&world, &on_mutate_sig](emp::AvidaGP & org, emp::Random & random) {

      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(20);
        org.RandomizeInst(pos, random);
      }
      on_mutate_sig.Trigger({{"substitution",num_muts}});
      return num_muts;
    });

  world.SetAutoMutate();

  // Setup the fitness function.
  std::function<double(emp::AvidaGP &)> fit_fun =
    [](emp::AvidaGP & org) {
      int count = 0;
      for (int i = 0; i < 16; i++) {
        org.ResetHardware();
        org.SetInput(0,i);
        org.SetOutput(0, -99999);
        org.Process(20);
        double score = 1.0 / (org.GetOutput(i) - (double) (i*i));
        if (score > 1000) {
          score = 1000;
        }
        count += score;
      }
      return (double) count;
    };


  world.SetFitFun(fit_fun);

  // emp::vector< std::function<double(const emp::AvidaGP &)> > fit_set(16);
  // for (size_t out_id = 0; out_id < 16; out_id++) {
  //   // Setup the fitness function.
  //   fit_set[out_id] = [out_id](const emp::AvidaGP & org) {
  //     return (double) -std::abs(org.GetOutput((int)out_id) - (double) (out_id * out_id));
  //   };
  // }

  // Build a random initial popoulation.
  for (size_t i = 0; i < 1; i++) {
    emp::AvidaGP cpu;
    cpu.PushRandom(random, 20);
    world.Inject(cpu.GetGenome());
  }

  for (size_t i = 0; i < 100; i++) {
      EliteSelect(world, 1, 1);
  }
  world.Update();

  // Do the run...
  for (size_t ud = 0; ud < 100; ud++) {
    // Update the status of all organisms.
    world.ResetHardware();
    world.Process(200);
    double fit0 = world.CalcFitnessID(0);
    std::cout << (ud+1) << " : " << 0 << " : " << fit0 << std::endl;

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(world, 2, 99);
    // LexicaseSelect(world, fit_set, POP_SIZE-1);
    // EcoSelect(world, fit_fun, fit_set, 100, 5, POP_SIZE-1);
    for (size_t i = 0; i < world.GetSize(); i++) {
      record_fit_sig.Trigger(i, world.CalcFitnessID(i));
      record_phen_sig.Trigger(i, phen_fun(world.GetOrg(i)));
    }

    world.Update();

  }

  // std::cout << std::endl;
  // world[0].PrintGenome();
  // std::cout << std::endl;
  // for (int i = 0; i < 16; i++) {
  //   std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  // }
  // std::cout << std::endl;
}



TEST_CASE("Test GetCanopy", "[evo]")
{
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  auto id1 = sys.AddOrg(1, nullptr, 0);
  auto id2 = sys.AddOrg(2, id1, 2);
  auto id3 = sys.AddOrg(3, id1, 3);
  auto id4 = sys.AddOrg(4, id2, 3);

  sys.RemoveOrg(id1, 3);
  sys.RemoveOrg(id2, 5);

  auto can_set = sys.GetCanopyExtantRoots(4);

  // Both 3 and 4 were alive at time point 4 so they are the canopy roots
  CHECK(can_set.size() == 2);
  CHECK(Has(can_set, id3));
  CHECK(Has(can_set, id4));

  can_set = sys.GetCanopyExtantRoots(2);

  // Both 3 and 4 were not alive at time point 2, so the canopy roots
  // will be 1 and 2. 
  CHECK(can_set.size() == 2);
  CHECK(Has(can_set, id1));
  CHECK(Has(can_set, id2));

  sys.RemoveOrg(id3, 7);

  can_set = sys.GetCanopyExtantRoots(2);

  // Only 4 is alive, but it wasn't alive at time point 2. 2 is the
  // only canopy root because even though 1 is alive, because 4's
  // lineage diverged from 1 when 2 was born. 
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id2));

  auto id5 = sys.AddOrg(5, id4, 8);
  sys.RemoveOrg(id4, 9);
  auto id6 = sys.AddOrg(6, id5, 10);
  sys.RemoveOrg(id5, 11);

  can_set = sys.GetCanopyExtantRoots(7);
  // Should only be 4 
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id4));

  can_set = sys.GetCanopyExtantRoots(9);
  // Should only be 5 
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id5));


  auto id7 = sys.AddOrg(7, id6, 12);
  auto id8 = sys.AddOrg(8, id7, 13);
  auto id9 = sys.AddOrg(9, id8, 14);
  auto id10 = sys.AddOrg(10, id9, 15);

  sys.RemoveOrg(id6, 20);
  sys.RemoveOrg(id7, 20);
  sys.RemoveOrg(id8, 20);
  sys.RemoveOrg(id9, 20);

  can_set = sys.GetCanopyExtantRoots(22);
  // Should only be 10 
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id10));

  can_set = sys.GetCanopyExtantRoots(14);
  // Should only be 9, even though others were alive 
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id9));

  can_set = sys.GetCanopyExtantRoots(13);
  // Should only be 8, because 9 wasn't born yet
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id8));

  can_set = sys.GetCanopyExtantRoots(11);
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id6));

  can_set = sys.GetCanopyExtantRoots(12);
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id7));

  can_set = sys.GetCanopyExtantRoots(9);
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id5));


  // auto id5 = sys.AddOrg(28, id2, 32);
  // std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  // auto id6 = sys.AddOrg(29, id5, 39);
  // std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  // auto id7 = sys.AddOrg(30, id1, 6);

}

// Tests from Shao 1990 tree balance paper
TEST_CASE("Tree balance", "[evo]") {
  emp::Systematics<int, int> tree1([](const int & i){return i;}, true, true, false, false);

  auto tree1org1 = tree1.AddOrg(1, nullptr);
  auto tree1org2 = tree1.AddOrg(2, tree1org1);
  auto tree1org3 = tree1.AddOrg(3, tree1org2);
  auto tree1org4 = tree1.AddOrg(4, tree1org3);
  auto tree1org5 = tree1.AddOrg(5, tree1org3);
  auto tree1org6 = tree1.AddOrg(6, tree1org2);
  auto tree1org7 = tree1.AddOrg(7, tree1org6);
  auto tree1org8 = tree1.AddOrg(8, tree1org6);
  auto tree1org9 = tree1.AddOrg(9, tree1org1);
  auto tree1org10 = tree1.AddOrg(10, tree1org9);
  auto tree1org11 = tree1.AddOrg(11, tree1org9);
  tree1.RemoveOrg(tree1org1);
  tree1.RemoveOrg(tree1org2);
  tree1.RemoveOrg(tree1org3);
  tree1.RemoveOrg(tree1org6);
  tree1.RemoveOrg(tree1org9);

  CHECK(tree1.SackinIndex() == 16);

  emp::Systematics<int, int> tree2([](const int & i){return i;}, true, true, false, false);

  auto tree2org1 = tree2.AddOrg(1, nullptr);
  auto tree2org2 = tree2.AddOrg(2, tree2org1);
  auto tree2org3 = tree2.AddOrg(3, tree2org2);
  auto tree2org4 = tree2.AddOrg(4, tree2org3);
  auto tree2org5 = tree2.AddOrg(5, tree2org3);
  auto tree2org6 = tree2.AddOrg(6, tree2org2);
  auto tree2org7 = tree2.AddOrg(7, tree2org1);
  auto tree2org8 = tree2.AddOrg(8, tree2org7);
  auto tree2org9 = tree2.AddOrg(9, tree2org7);
  auto tree2org10 = tree2.AddOrg(10, tree2org9);
  auto tree2org11 = tree2.AddOrg(11, tree2org9);

  tree2.RemoveOrg(tree2org1);
  tree2.RemoveOrg(tree2org2);
  tree2.RemoveOrg(tree2org3);
  tree2.RemoveOrg(tree2org7);
  tree2.RemoveOrg(tree2org9);

  CHECK(tree2.SackinIndex() == 16);

  emp::Systematics<int, int> tree3([](const int & i){return i;}, true, true, false, false);

  auto tree3org1 = tree3.AddOrg(1, nullptr);
  auto tree3org2 = tree3.AddOrg(2, tree3org1);
  auto tree3org3 = tree3.AddOrg(3, tree3org2);
  auto tree3org4 = tree3.AddOrg(4, tree3org2);
  auto tree3org5 = tree3.AddOrg(5, tree3org4);
  auto tree3org6 = tree3.AddOrg(6, tree3org4);
  auto tree3org7 = tree3.AddOrg(7, tree3org6);
  auto tree3org8 = tree3.AddOrg(8, tree3org6);
  auto tree3org9 = tree3.AddOrg(9, tree3org1);
  auto tree3org10 = tree3.AddOrg(10, tree3org9);
  auto tree3org11 = tree3.AddOrg(11, tree3org9);

  tree3.RemoveOrg(tree3org1);
  tree3.RemoveOrg(tree3org2);
  tree3.RemoveOrg(tree3org4);
  tree3.RemoveOrg(tree3org6);
  tree3.RemoveOrg(tree3org9);

  CHECK(tree3.SackinIndex() == 17);

  emp::Systematics<int, int> tree29([](const int & i){return i;}, true, true, false, false);

  auto tree29org1 = tree29.AddOrg(1, nullptr);
  auto tree29org2 = tree29.AddOrg(2, tree29org1);
  auto tree29org3 = tree29.AddOrg(3, tree29org1);
  auto tree29org4 = tree29.AddOrg(4, tree29org3);
  auto tree29org5 = tree29.AddOrg(5, tree29org3);
  auto tree29org6 = tree29.AddOrg(6, tree29org3);
  auto tree29org7 = tree29.AddOrg(7, tree29org3);
  auto tree29org8 = tree29.AddOrg(8, tree29org3);

  tree29.RemoveOrg(tree29org1);
  tree29.RemoveOrg(tree29org3);

  CHECK(tree29.SackinIndex() == 11);

  emp::Systematics<int, int> tree30([](const int & i){return i;}, true, true, false, false);

  auto tree30org1 = tree30.AddOrg(1, nullptr);
  auto tree30org2 = tree30.AddOrg(2, tree30org1);
  auto tree30org3 = tree30.AddOrg(3, tree30org1);
  auto tree30org4 = tree30.AddOrg(4, tree30org1);
  auto tree30org5 = tree30.AddOrg(5, tree30org4);
  auto tree30org6 = tree30.AddOrg(6, tree30org4);
  auto tree30org7 = tree30.AddOrg(7, tree30org4);
  auto tree30org8 = tree30.AddOrg(8, tree30org4);

  tree30.RemoveOrg(tree30org1);
  tree30.RemoveOrg(tree30org4);

  CHECK(tree30.SackinIndex() == 10);

  emp::Systematics<int, int> tree31([](const int & i){return i;}, true, true, false, false);

  auto tree31org1 = tree31.AddOrg(1, nullptr);
  auto tree31org2 = tree31.AddOrg(2, tree31org1);
  auto tree31org3 = tree31.AddOrg(3, tree31org1);
  auto tree31org4 = tree31.AddOrg(4, tree31org1);
  auto tree31org5 = tree31.AddOrg(5, tree31org1);
  auto tree31org6 = tree31.AddOrg(6, tree31org5);
  auto tree31org7 = tree31.AddOrg(7, tree31org5);
  auto tree31org8 = tree31.AddOrg(8, tree31org5);

  tree31.RemoveOrg(tree31org1);
  tree31.RemoveOrg(tree31org5);

  CHECK(tree31.SackinIndex() == 9);

  emp::Systematics<int, int> tree32([](const int & i){return i;}, true, true, false, false);

  auto tree32org1 = tree32.AddOrg(1, nullptr);
  auto tree32org2 = tree32.AddOrg(2, tree32org1);
  auto tree32org3 = tree32.AddOrg(3, tree32org1);
  auto tree32org4 = tree32.AddOrg(4, tree32org1);
  auto tree32org5 = tree32.AddOrg(5, tree32org1);
  auto tree32org6 = tree32.AddOrg(6, tree32org1);
  auto tree32org7 = tree32.AddOrg(7, tree32org6);
  auto tree32org8 = tree32.AddOrg(8, tree32org6);

  tree32.RemoveOrg(tree32org1);
  tree32.RemoveOrg(tree32org6);

  CHECK(tree32.SackinIndex() == 8);

  emp::Systematics<int, int> tree33([](const int & i){return i;}, true, true, false, false);

  auto tree33org1 = tree33.AddOrg(1, nullptr);
  auto tree33org2 = tree33.AddOrg(2, tree33org1);
  auto tree33org3 = tree33.AddOrg(3, tree33org1);
  auto tree33org4 = tree33.AddOrg(4, tree33org1);
  auto tree33org5 = tree33.AddOrg(5, tree33org1);
  auto tree33org6 = tree33.AddOrg(6, tree33org1);
  auto tree33org7 = tree33.AddOrg(7, tree33org1);

  tree33.RemoveOrg(tree33org1);
  CHECK(tree33.SackinIndex() == 6);

  // From CollessLike metric paper
  emp::Systematics<int, int> treecl([](const int & i){return i;}, true, true, false, false);  
  auto treeclorg1 = treecl.AddOrg(1, nullptr);
  auto treeclorg2 = treecl.AddOrg(2, treeclorg1);
  auto treeclorg3 = treecl.AddOrg(3, treeclorg1);
  auto treeclorg4 = treecl.AddOrg(4, treeclorg2);
  auto treeclorg5 = treecl.AddOrg(5, treeclorg2);
  auto treeclorg6 = treecl.AddOrg(6, treeclorg2);
  auto treeclorg7 = treecl.AddOrg(7, treeclorg2);
  auto treeclorg8 = treecl.AddOrg(8, treeclorg2);
  auto treeclorg9 = treecl.AddOrg(9, treeclorg3);
  auto treeclorg10 = treecl.AddOrg(10, treeclorg3);
  auto treeclorg11 = treecl.AddOrg(11, treeclorg10);
  auto treeclorg12 = treecl.AddOrg(12, treeclorg10);

  treecl.RemoveOrg(treeclorg1);
  treecl.RemoveOrg(treeclorg2);
  treecl.RemoveOrg(treeclorg3);
  treecl.RemoveOrg(treeclorg10);

  CHECK(treecl.SackinIndex() == 18);
  CHECK(treecl.CollessLikeIndex() == Approx(1.746074));
}