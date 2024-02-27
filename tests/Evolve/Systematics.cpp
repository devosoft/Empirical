/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Systematics.cpp
 */

#include <filesystem>
#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#ifndef NDEBUG
  #define TDEBUG
#endif

#include "emp/base/vector.hpp"
#include "emp/Evolve/SystematicsAnalysis.hpp"
#include "emp/Evolve/Systematics.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/Evolve/World_output.hpp"
#include "emp/hardware/AvidaGP.hpp"

TEST_CASE("Test Systematics", "[Evolve]") {
  // Taxon
  emp::Taxon<std::string> tx(0, "a");
  CHECK(tx.GetID() == 0);
  CHECK(tx.GetParent() == nullptr);
  CHECK(tx.GetInfo() == "a");
  CHECK(tx.GetNumOrgs() == 0);
  CHECK(tx.GetTotOrgs() == 0);
  tx.AddOrg();
  CHECK(tx.GetNumOrgs() == 1);
  tx.RemoveOrg();
  CHECK(tx.GetNumOrgs() == 0);
  CHECK(tx.GetTotOrgs() == 1);
  CHECK(tx.GetTotalOffspring() == 0);

  emp::Ptr< emp::Taxon<std::string, emp::datastruct::no_data> > parentPtr(&tx);
  emp::Taxon<std::string> tx_1(1, "b", parentPtr);
  CHECK(tx_1.GetParent() == parentPtr);
  tx_1.AddTotalOffspring();
  CHECK(tx_1.GetTotalOffspring() == 1);
  CHECK(tx.GetTotalOffspring() == 1);

  // Systematics
  std::function<std::string(double &)> calc_taxon = [](double & o){ return o > 50.0 ? "large" : "small"; };
  emp::Systematics<double, std::string> sys1(calc_taxon);
  CHECK(sys1.GetTrackSynchronous() == false);
  CHECK(sys1.GetNumAncestors() == 0);
  CHECK(sys1.GetNumActive() == 0);
  CHECK(sys1.GetNumOutside() == 0);
  CHECK(sys1.GetTreeSize() == 0);
  CHECK(sys1.GetNumTaxa() == 0);

  sys1.SetTrackSynchronous(true);
  CHECK(sys1.GetTrackSynchronous() == true);
  sys1.AddOrg(15.0, {0,0});
  CHECK(sys1.CalcDiversity() == 0);
  CHECK(sys1.GetNumActive() == 1);
  CHECK(sys1.GetTaxonAt({0,0})->GetInfo() == "small");
  CHECK(sys1.IsTaxonAt({0,0}));
  sys1.AddOrg(56.0, {1,1});
  CHECK(sys1.GetNumActive() == 2);
  CHECK(sys1.CalcDiversity() == 1);
  CHECK(sys1.GetTaxonAt({1,1})->GetInfo() == "large");
  CHECK(sys1.IsTaxonAt({1,1}));
  sys1.RemoveOrg({1,1});
  CHECK(!sys1.IsTaxonAt({1,1}));
  CHECK(sys1.GetNumActive() == 1);
  sys1.AddOrg(56.0, {1,0});
  CHECK(sys1.IsTaxonAt({1,0}));
  CHECK(!sys1.RemoveOrg({1,0}));
  CHECK(!sys1.IsTaxonAt({1,0}));

  // Base setters and getters
  CHECK(sys1.GetStoreActive() == true);
  CHECK(sys1.GetStoreAncestors() == true);
  CHECK(sys1.GetStoreOutside() == false);
  CHECK(sys1.GetArchive() == true);
  CHECK(sys1.GetStorePosition() == true);
  sys1.SetStoreActive(false);
  CHECK(sys1.GetStoreActive() == false);
  sys1.SetStoreAncestors(false);
  CHECK(sys1.GetStoreAncestors() == false);
  sys1.SetStoreOutside(true);
  CHECK(sys1.GetStoreOutside() == true);
  sys1.SetArchive(false);
  CHECK(sys1.GetArchive() == false);
  sys1.SetStorePosition(false);
  CHECK(sys1.GetStorePosition() == false);

  #ifndef NDEBUG
  sys1.AddDeleteriousStepDataNode();
  CHECK(emp::assert_last_fail);
  emp::assert_clear();

  sys1.AddVolatilityDataNode();
  CHECK(emp::assert_last_fail);
  emp::assert_clear();

  sys1.AddUniqueTaxaDataNode();
  CHECK(emp::assert_last_fail);
  emp::assert_clear();

  sys1.AddMutationCountDataNode();
  CHECK(emp::assert_last_fail);
  emp::assert_clear();
  #endif

  // Analysis
  using my_taxon = emp::Taxon<std::string, emp::datastruct::mut_landscape_info<double>>;
  //emp::Systematics<double, std::string, emp::datastruct::mut_landscape_info> sys2(calc_taxon)
  my_taxon taxon1(1, "medium");
  emp::Ptr<my_taxon> ptr1 = &taxon1;
  CHECK(emp::LineageLength(ptr1) == 1);
  my_taxon taxon2(1, "medium", ptr1);
  emp::Ptr<my_taxon> ptr2 = &taxon2;
  CHECK(emp::LineageLength(ptr1) == 1);
  CHECK(emp::LineageLength(ptr2) == 2);
  std::unordered_map<std::string, int> muts;
  muts["short"] = 12;
  muts["tall"] = 3;
  taxon2.GetData().RecordMutation(muts);
  CHECK(taxon2.GetData().mut_counts.size() == 2);
  CHECK(taxon2.GetData().mut_counts["tall"] == 3);

  emp::vector<std::string> types;
  types.push_back("tall");
  types.push_back("short");
  CHECK(emp::CountMuts(ptr2, types) == 15);
  CHECK(emp::CountMutSteps(ptr2, types) == 2);
  CHECK(emp::CountMutSteps(ptr2, "short") == 1);
  muts["short"] = 4;
  taxon1.GetData().RecordMutation(muts);
  CHECK(emp::CountMuts(ptr1, "short") == 4);
  CHECK(emp::CountMuts(ptr2, "short") == 16);
  CHECK(emp::CountMutSteps(ptr1, "short") == 1);
  CHECK(emp::CountMutSteps(ptr2, "short") == 2);

  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  // std::cout << "\nAddOrg 25 (id1, no parent)\n";
  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  // std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  // std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  // std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  // std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  // std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id5);
  // std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  sys.SetUpdate(6);
  auto id7 = sys.AddOrg(30, id1);

  CHECK(*id1 < *id2);
  CHECK(sys.Parent(id2) == id1);

  // std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);

  double mpd = sys.GetMeanPairwiseDistance();
  // std::cout << "MPD: " << mpd <<std::endl;
  CHECK(mpd == Approx(2.8));

  double sd = sys.GetSumDistance();
  // std::cout << "MPD: " << mpd <<std::endl;
  CHECK(sd == Approx(74.0));

  // std::cout << "\nAddOrg 31 (id8; parent id7)\n";
  sys.SetUpdate(11);
  auto id8 = sys.AddOrg(31, id7);
  // std::cout << "\nAddOrg 32 (id9; parent id8)\n";
  sys.SetUpdate(19);
  auto id9 = sys.AddOrg(32, id8);


  CHECK(sys.GetEvolutionaryDistinctiveness(id3, 10) == 10);
  CHECK(sys.GetEvolutionaryDistinctiveness(id4, 25) == 21);
  CHECK(sys.GetEvolutionaryDistinctiveness(id5, 32) == 15);
  CHECK(sys.GetEvolutionaryDistinctiveness(id6, 39) == 22);
  CHECK(sys.GetEvolutionaryDistinctiveness(id6, 45) == 28);
  CHECK(sys.GetEvolutionaryDistinctiveness(id9, 19) == 12.5);

  // std::cout << "\nAddOrg 33 (id10; parent id8)\n";
  auto id10 = sys.AddOrg(33, id8);

  sys.RemoveOrg(id7);
  sys.RemoveOrg(id8);

  CHECK(sys.GetEvolutionaryDistinctiveness(id9, 19) == 13.5);
  CHECK(sys.GetEvolutionaryDistinctiveness(id10, 19) == 13.5);

  sys.RemoveOrg(id10);

  CHECK(sys.GetEvolutionaryDistinctiveness(id9, 19) == 19);


  // std::cout << "\nAddOrg 34 (id11; parent id9)\n";
  sys.SetUpdate(22);
  auto id11 = sys.AddOrg(34, id9);
  // std::cout << "\nAddOrg 35 (id12; parent id10)\n";
  sys.SetUpdate(23);
  auto id12 = sys.AddOrg(35, id11);

  sys.RemoveOrg(id9);

  CHECK(sys.GetEvolutionaryDistinctiveness(id11, 26) == 13);
  CHECK(sys.GetEvolutionaryDistinctiveness(id12, 26) == 15);

  // std::cout << "\nAddOrg 36 (id13; parent id12)\n";
  sys.SetUpdate(27);
  auto id13 = sys.AddOrg(36, id12);
  // std::cout << "\nAddOrg 37 (id14; parent id13)\n";
  sys.SetUpdate(30);
  auto id14 = sys.AddOrg(37, id13);

  sys.RemoveOrg(id13);

  CHECK(sys.GetEvolutionaryDistinctiveness(id14, 33) == Approx(17.833333));

  // std::cout << "\nAddOrg 38 (id15; parent id14)\n";
  sys.SetUpdate(33);
  auto id15 = sys.AddOrg(38, id14);

  sys.RemoveOrg(id14);

  CHECK(sys.GetEvolutionaryDistinctiveness(id15, 33) == Approx(17.833333));

  // std::cout << "\nAddOrg 39 (id16; parent id11)\n";
  sys.SetUpdate(35);
  auto id16 = sys.AddOrg(39, id11);
  // std::cout << "\nAddOrg 40 (id17; parent id11)\n";
  auto id17 = sys.AddOrg(40, id11);

  CHECK(sys.GetEvolutionaryDistinctiveness(id16, 35) == Approx(17.4));
  CHECK(sys.GetEvolutionaryDistinctiveness(id17, 35) == Approx(17.4));

  // std::cout << "\nAddOrg 41 (id18; parent id17)\n";
  sys.SetUpdate(36);
  auto id18 = sys.AddOrg(41, id17);

  CHECK(sys.GetEvolutionaryDistinctiveness(id18, 37) == Approx(12.1666667));

  CHECK(sys.GetTaxonDistinctiveness(id18) == Approx(1.0/6.0));
  CHECK(sys.GetBranchesToRoot(id18) == 1);
  CHECK(sys.GetDistanceToRoot(id18) == 6);

  std::cout << "\nAddOrg 42 (id19; parent id17)\n";
  sys.SetUpdate(37);
  auto id19 = sys.AddOrg(42, id17);
  CHECK(sys.GetBranchesToRoot(id19) == 2);
  CHECK(sys.GetDistanceToRoot(id19) == 6);
  CHECK(sys.GetTaxonDistinctiveness(id19) == Approx(1.0/6.0));

  CHECK(sys.GetTaxonDistinctiveness(id15) == Approx(1.0/8.0));
  CHECK(sys.GetBranchesToRoot(id15) == 1);
  CHECK(sys.GetDistanceToRoot(id15) == 8);
  CHECK(sys.GetPhylogeneticDiversity() == 17);
  CHECK(sys.GetAveDepth() == Approx(4.272727));

  std::cout << "id1 = " << id1 << std::endl;
  std::cout << "id2 = " << id2 << std::endl;
  std::cout << "id3 = " << id3 << std::endl;
  std::cout << "id4 = " << id4 << std::endl;

  std::stringstream result;

  sys.PrintLineage(id4, result);
  sys.PrintStatus();

  CHECK(result.str() == "Lineage:\n27\n-10\n25\n");

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

  CHECK(sys.GetMaxDepth() == 8);

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

TEST_CASE("Test not tracking ancestors", "[Evolve]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, false, false, false);

  // std::cout << "\nAddOrg 25 (id1, no parent)\n";
  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  // std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  // std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  // std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  // std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  // std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id5);
  // std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  sys.SetUpdate(6);
  auto id7 = sys.AddOrg(30, id1);


  // std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);

  // std::cout << "\nAddOrg 31 (id8; parent id7)\n";
  sys.SetUpdate(11);
  auto id8 = sys.AddOrg(31, id7);
  // std::cout << "\nAddOrg 32 (id9; parent id8)\n";
  sys.SetUpdate(19);
  auto id9 = sys.AddOrg(32, id8);

  // std::cout << "\nAddOrg 33 (id10; parent id8)\n";
  auto id10 = sys.AddOrg(33, id8);

  sys.RemoveOrg(id7);
  sys.RemoveOrg(id8);

  sys.RemoveOrg(id10);

  // std::cout << "\nAddOrg 34 (id11; parent id9)\n";
  sys.SetUpdate(22);
  auto id11 = sys.AddOrg(34, id9);
  // std::cout << "\nAddOrg 35 (id12; parent id10)\n";
  sys.SetUpdate(23);
  auto id12 = sys.AddOrg(35, id11);

  sys.RemoveOrg(id9);

  // std::cout << "\nAddOrg 36 (id13; parent id12)\n";
  sys.SetUpdate(27);
  auto id13 = sys.AddOrg(36, id12);
  // std::cout << "\nAddOrg 37 (id14; parent id13)\n";
  sys.SetUpdate(30);
  auto id14 = sys.AddOrg(37, id13);

  sys.RemoveOrg(id13);

  // std::cout << "\nAddOrg 38 (id15; parent id14)\n";
  sys.SetUpdate(33);
  auto id15 = sys.AddOrg(38, id14);

  sys.RemoveOrg(id14);

  // std::cout << "\nAddOrg 39 (id16; parent id11)\n";
  sys.SetUpdate(35);
  auto id16 = sys.AddOrg(39, id11);
  // std::cout << "\nAddOrg 40 (id17; parent id11)\n";
  auto id17 = sys.AddOrg(40, id11);

  // std::cout << "\nAddOrg 41 (id18; parent id17)\n";
  sys.SetUpdate(36);
  auto id18 = sys.AddOrg(41, id17);

  std::cout << "\nAddOrg 42 (id19; parent id17)\n";
  sys.SetUpdate(37);
  auto id19 = sys.AddOrg(42, id17);

  CHECK(id17->GetTotalOffspring() > 0);

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

TEST_CASE("Test Data Struct", "[evo]") {

  emp::Ptr<emp::Systematics<int, int, emp::datastruct::mut_landscape_info<int> >> sys;
  sys.New([](const int & i){return i;}, true, true, true, false);
  sys->AddMutationCountDataNode();
  sys->AddVolatilityDataNode();
  sys->AddUniqueTaxaDataNode();

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
  std::unordered_map<std::string, int> muts;
  muts["substitution"] = 1;
  id5->GetData().RecordMutation(muts);
  id5->GetData().RecordFitness(2);
  id5->GetData().RecordPhenotype(6);

  CHECK(id5->GetData().GetPhenotype() == 6);
  CHECK(id5->GetData().GetFitness() == 2);

  CHECK(CountMuts(id4) == 3);
  CHECK(CountDeleteriousSteps(id4) == 1);
  CHECK(CountPhenotypeChanges(id4) == 1);
  CHECK(CountUniquePhenotypes(id4) == 2);
  CHECK(LineageLength(id4) == 3);

  CHECK(CountMuts(id3) == 5);
  CHECK(CountDeleteriousSteps(id3) == 1);
  CHECK(CountPhenotypeChanges(id3) == 0);
  CHECK(CountUniquePhenotypes(id3) == 1);
  CHECK(LineageLength(id3) == 2);

  CHECK(CountMuts(id5) == 4);
  CHECK(CountDeleteriousSteps(id5) == 2);
  CHECK(CountPhenotypeChanges(id5) == 2);
  CHECK(CountUniquePhenotypes(id5) == 2);
  CHECK(LineageLength(id5) == 4);

  CHECK(FindDominant(*sys) == id4);

  sys->GetDataNode("mutation_count")->PullData();
  CHECK(sys->GetDataNode("mutation_count")->GetMean() == Approx(2.8));

  sys->GetDataNode("volatility")->PullData();
  CHECK(sys->GetDataNode("volatility")->GetMean() == Approx(0.6));

  sys->GetDataNode("unique_taxa")->PullData();
  CHECK(sys->GetDataNode("unique_taxa")->GetMean() == Approx(1.4));


  sys.Delete();

  emp::Ptr<emp::Systematics<int, int, emp::datastruct::fitness >> sys2;
  sys2.New([](const int & i){return i;}, true, true, true, false);
  sys2->AddDeleteriousStepDataNode();

  auto new_tax = sys2->AddOrg(1, nullptr);
  new_tax->GetData().RecordFitness(2);
  CHECK(new_tax->GetData().GetFitness() == 2);
  new_tax->GetData().RecordFitness(4);
  CHECK(new_tax->GetData().GetFitness() == 3);

  emp::datastruct::fitness fit_data;
  fit_data.RecordFitness(5);
  new_tax->SetData(fit_data);
  CHECK(new_tax->GetData().GetFitness() == 5);

  auto tax2 = sys2->AddOrg(2, new_tax);
  tax2->GetData().RecordFitness(1);

  sys->GetDataNode("deleterious_steps")->PullData();
  CHECK(sys->GetDataNode("deleterious_steps")->GetMean() == Approx(.5));


  sys2.Delete();


}

TEST_CASE("World systematics integration", "[evo]") {

  std::function<void(emp::Ptr<emp::Taxon<emp::vector<int>, emp::datastruct::mut_landscape_info<int>>>, emp::vector<int> &)> setup_phenotype = [](emp::Ptr<emp::Taxon<emp::vector<int>, emp::datastruct::mut_landscape_info<int>>> tax, emp::vector<int> & org){
    tax->GetData().phenotype = emp::Sum(tax->GetInfo());
  };

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

  sys->OnNew(setup_phenotype);
  world.InjectAt(emp::vector<int>({1,2,3}), 0);

  CHECK(sys->GetTaxonAt(0)->GetData().phenotype == 6);
  sys->GetTaxonAt(0)->GetData().RecordPhenotype(10);
  CHECK(sys->GetTaxonAt(0)->GetData().phenotype == 10);

  sys->GetTaxonAt(0)->GetData().RecordFitness(2);

  std::unordered_map<std::string, int> mut_counts;
  mut_counts["substitution"] = 3;

  emp::vector<int> new_org({4,2,3});
  auto old_taxon = sys->GetTaxonAt(0);
  world.DoBirth(new_org,0);

  CHECK(old_taxon->GetNumOrgs() == 0);
  CHECK(old_taxon->GetNumOff() == 1);
  CHECK(sys->GetTaxonAt(0)->GetParent()->GetData().phenotype == 10);
  CHECK((*sys->GetActive().begin())->GetNumOrgs() == 1);

}

template <typename WORLD_TYPE>
emp::DataFile AddDominantFile(WORLD_TYPE & world){
  using mut_count_t [[maybe_unused]] = std::unordered_map<std::string, int>;
  using data_t = emp::datastruct::mut_landscape_info<emp::vector<double>>;
  using org_t = emp::AvidaGP;
  using systematics_t = emp::Systematics<org_t, org_t::genome_t, data_t>;


  auto & file = world.SetupFile("dominant.csv");

  std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};
  std::function<int(void)> dom_mut_count = [&world](){
    emp::Ptr<emp::SystematicsBase<org_t>> sys = world.GetSystematics(0);
    emp::Ptr<systematics_t> full_sys = sys.DynamicCast<systematics_t>();
    if (full_sys->GetNumActive() > 0) {
      return emp::CountMuts(emp::FindDominant(*full_sys));
    }
    return 0;
  };
  std::function<int(void)> dom_del_step = [&world](){
    emp::Ptr<emp::SystematicsBase<org_t>> sys = world.GetSystematics(0);
    emp::Ptr<systematics_t> full_sys = sys.DynamicCast<systematics_t>();
    if (full_sys->GetNumActive() > 0) {
      return emp::CountDeleteriousSteps(emp::FindDominant(*full_sys));
    }
    return 0;
  };
  std::function<size_t(void)> dom_phen_vol = [&world](){
    emp::Ptr<emp::SystematicsBase<org_t>> sys = world.GetSystematics(0);
    emp::Ptr<systematics_t> full_sys = sys.DynamicCast<systematics_t>();
    if (full_sys->GetNumActive() > 0) {
      return emp::CountPhenotypeChanges(emp::FindDominant(*full_sys));
    }
    return 0;
  };
  std::function<size_t(void)> dom_unique_phen = [&world](){
    emp::Ptr<emp::SystematicsBase<org_t>> sys = world.GetSystematics(0);
    emp::Ptr<systematics_t> full_sys = sys.DynamicCast<systematics_t>();
    if (full_sys->GetNumActive() > 0) {
      return emp::CountUniquePhenotypes(emp::FindDominant(*full_sys));
    }
    return 0;
  };
  std::function<size_t(void)> lin_len = [&world](){
    emp::Ptr<emp::SystematicsBase<org_t>> sys = world.GetSystematics(0);
    emp::Ptr<systematics_t> full_sys = sys.DynamicCast<systematics_t>();
    if (full_sys->GetNumActive() > 0) {
      return emp::LineageLength(emp::FindDominant(*full_sys));
    }
    return 0;
  };

  file.AddFun(get_update, "update", "Update");
  file.AddFun(dom_mut_count, "dominant_mutation_count", "sum of mutations along dominant organism's lineage");
  file.AddFun(dom_del_step, "dominant_deleterious_steps", "count of deleterious steps along dominant organism's lineage");
  file.AddFun(dom_phen_vol, "dominant_phenotypic_volatility", "count of changes in phenotype along dominant organism's lineage");
  file.AddFun(dom_unique_phen, "dominant_unique_phenotypes", "count of unique phenotypes along dominant organism's lineage");
  file.AddFun(lin_len, "lineage_length", "number of taxa dominant organism's lineage");
  file.PrintHeaderKeys();
  return file;
}

// Integration test for using multiple systematics managers in a world and recording data
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

  std::function<void(emp::Ptr<gene_systematics_t::taxon_t>, emp::AvidaGP&)> check_update = [&gene_sys, &world](emp::Ptr<gene_systematics_t::taxon_t> tax, emp::AvidaGP & org){
    CHECK(tax->GetOriginationTime() == gene_sys->GetUpdate());
    CHECK(tax->GetOriginationTime() == world.GetUpdate());
    CHECK(tax->GetNumOff() == 0);
  };

  gene_sys->OnNew(check_update);

  std::function<void(emp::Ptr<gene_systematics_t::taxon_t>)> extinction_checks = [&gene_sys, &world](emp::Ptr<gene_systematics_t::taxon_t> tax){
    CHECK(tax->GetDestructionTime() == gene_sys->GetUpdate());
    CHECK(tax->GetDestructionTime() == world.GetUpdate());
    CHECK(tax->GetNumOrgs() == 0);
  };

  gene_sys->OnExtinct(extinction_checks);

  std::function<void(emp::Ptr<gene_systematics_t::taxon_t>)> prune_checks = [&world](emp::Ptr<gene_systematics_t::taxon_t> tax){
    CHECK(tax->GetNumOrgs() == 0);
    CHECK(tax->GetNumOff() == 0);
    CHECK(tax->GetOriginationTime() <= world.GetUpdate());
    CHECK(tax->GetDestructionTime() <= world.GetUpdate());
  };

  gene_sys->OnPrune(prune_checks);


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

  emp::Ptr<emp::SystematicsBase<org_t>> sys0 = world.GetSystematics(0);
  emp::Ptr<gene_systematics_t> sys0_cast = sys0.DynamicCast<gene_systematics_t>();
  std::function<void(emp::Ptr<gene_systematics_t::taxon_t>, emp::AvidaGP&)> capture_mut_fun = [&last_mutation](emp::Ptr<gene_systematics_t::taxon_t> tax, emp::AvidaGP & org){
    tax->GetData().RecordMutation(last_mutation);
  };
  sys0_cast->OnNew(capture_mut_fun);

  world.SetupSystematicsFile().SetTimingRepeat(1);
  world.SetupFitnessFile().SetTimingRepeat(1);
  world.SetupPopulationFile().SetTimingRepeat(1);
  emp::AddPhylodiversityFile(world, 0, "genotype_phylodiversity.csv").SetTimingRepeat(1);
  emp::AddPhylodiversityFile(world, 1, "phenotype_phylodiversity.csv").SetTimingRepeat(1);
  emp::AddLineageMutationFile(world).SetTimingRepeat(1);
  AddDominantFile(world).SetTimingRepeat(1);
  // emp::AddMullerPlotFile(world).SetTimingOnce(1);


  // Setup the mutation function.
  world.SetMutFun( [&world, &on_mutate_sig](emp::AvidaGP & org, emp::Random & random) {
      (void) world;
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

  // Build a random initial population.
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
    TournamentSelect(world, 2, 100);

    for (size_t i = 0; i < world.GetSize(); i++) {
      record_fit_sig.Trigger(i, world.CalcFitnessID(i));
      record_phen_sig.Trigger(i, phen_fun(world.GetOrg(i)));
    }

    world.Update();
    CHECK(world.GetUpdate() == gene_sys->GetUpdate());
    CHECK(world.GetUpdate() == phen_sys->GetUpdate());
    CHECK(gene_sys->GetTaxonAt(0)->GetOriginationTime() <= world.GetUpdate());
  }
}

TEST_CASE("Test GetCanopy", "[evo]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(1, nullptr);
  sys.SetUpdate(2);
  auto id2 = sys.AddOrg(2, id1);
  sys.SetUpdate(3);
  auto id3 = sys.AddOrg(3, id1);
  auto id4 = sys.AddOrg(4, id2);

  sys.RemoveOrg(id1);
  sys.SetUpdate(5);
  sys.RemoveOrg(id2);

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

  sys.SetUpdate(7);
  sys.RemoveOrg(id3);

  can_set = sys.GetCanopyExtantRoots(2);

  // Only 4 is alive, but it wasn't alive at time point 2. 2 is the
  // only canopy root because even though 1 is alive, because 4's
  // lineage diverged from 1 when 2 was born.
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id2));

  sys.SetUpdate(8);
  auto id5 = sys.AddOrg(5, id4);
  sys.SetUpdate(9);
  sys.RemoveOrg(id4);
  sys.SetUpdate(10);
  auto id6 = sys.AddOrg(6, id5);
  sys.SetUpdate(11);
  sys.RemoveOrg(id5);

  can_set = sys.GetCanopyExtantRoots(7);
  // Should only be 4
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id4));

  can_set = sys.GetCanopyExtantRoots(9);
  // Should only be 5
  CHECK(can_set.size() == 1);
  CHECK(Has(can_set, id5));

  sys.SetUpdate(12);
  auto id7 = sys.AddOrg(7, id6);
  sys.SetUpdate(13);
  auto id8 = sys.AddOrg(8, id7);
  sys.SetUpdate(14);
  auto id9 = sys.AddOrg(9, id8);
  sys.SetUpdate(15);
  auto id10 = sys.AddOrg(10, id9);

  sys.SetUpdate(20);
  sys.RemoveOrg(id6);
  sys.RemoveOrg(id7);
  sys.RemoveOrg(id8);
  sys.RemoveOrg(id9);

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

// Test that MRCA is properly updated when the MRCA is alive and then dies,
// causing a new taxon to be MRCA
TEST_CASE("Dieing MRCA", "[evo]") {
  emp::Systematics<int, int> tree([](const int & i){return i;}, true, true, false, false);
  CHECK(!tree.GetTrackSynchronous());

  // std::cout << "\nAddOrg 25 (id1, no parent)\n";
  tree.SetUpdate(0);
  auto id1 = tree.AddOrg(25, nullptr);
  // std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  tree.SetUpdate(6);
  auto id2 = tree.AddOrg(-10, id1);
  // std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  tree.SetUpdate(10);
  auto id3 = tree.AddOrg(26, id1);
  // std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  tree.SetUpdate(25);
  auto id4 = tree.AddOrg(27, id2);
  // std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  tree.SetUpdate(32);
  auto id5 = tree.AddOrg(28, id2);
  // std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  tree.SetUpdate(39);
  auto id6 = tree.AddOrg(29, id5);
  // std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  tree.SetUpdate(6);
  auto id7 = tree.AddOrg(30, id1);

  CHECK(tree.GetMRCA() == id1);
  tree.RemoveOrg(id7);
  tree.RemoveOrg(id3);
  tree.RemoveOrg(id2);
  CHECK(tree.GetMRCA() == id1);
  tree.RemoveOrg(id1);
  CHECK(tree.GetMRCA() == id2);
  tree.RemoveOrg(id4);
  CHECK(tree.GetMRCA() == id5);
  tree.RemoveOrg(id5);
  CHECK(tree.GetMRCA() == id6);
}

TEST_CASE("Test RemoveBefore", "[Evolve]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, false, false);

  // std::cout << "\nAddOrg 25 (id1, no parent)\n";
  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  // std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  // std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  // std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  // std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  // std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id5);
  // std::cout << "\nAddOrg 30 (id7; parent id1)\n";
  sys.SetUpdate(6);
  auto id7 = sys.AddOrg(30, id1);
  sys.SetUpdate(33);
  auto id8 = sys.AddOrg(2, id3);
  auto id9 = sys.AddOrg(4, id8);
  sys.SetUpdate(34);
  auto id10 = sys.AddOrg(5, id9);

  sys.SetUpdate(40);
  sys.RemoveOrg(id1);
  sys.SetUpdate(41);
  sys.RemoveOrg(id2);
  sys.SetUpdate(40);
  sys.RemoveOrg(id9);
  sys.SetUpdate(60);
  sys.RemoveOrg(id8);

  CHECK(emp::Has(sys.GetAncestors(), id1));
  CHECK(emp::Has(sys.GetAncestors(), id2));

  sys.RemoveBefore(50);

  CHECK(!emp::Has(sys.GetAncestors(), id1));
  CHECK(!emp::Has(sys.GetAncestors(), id2));
  CHECK(emp::Has(sys.GetAncestors(), id9));
  CHECK(emp::Has(sys.GetActive(), id3));
  CHECK(emp::Has(sys.GetActive(), id4));
  CHECK(emp::Has(sys.GetActive(), id5));
  CHECK(emp::Has(sys.GetActive(), id6));
  CHECK(emp::Has(sys.GetActive(), id7));
  CHECK(emp::Has(sys.GetAncestors(), id8));

  sys.RemoveBefore(70);
  CHECK(!emp::Has(sys.GetActive(), id8));
  CHECK(!emp::Has(sys.GetActive(), id9));

}

TEST_CASE("Test Snapshot", "[Evolve]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id5);
  sys.SetUpdate(6);
  auto id7 = sys.AddOrg(30, id1);
  sys.SetUpdate(33);
  auto id8 = sys.AddOrg(2, id3);
  auto id9 = sys.AddOrg(4, id8);
  sys.SetUpdate(34);
  auto id10 = sys.AddOrg(5, id9);

  sys.SetUpdate(40);
  sys.RemoveOrg(id1);
  sys.SetUpdate(41);
  sys.RemoveOrg(id2);
  sys.SetUpdate(40);
  sys.RemoveOrg(id9);
  sys.SetUpdate(60);
  sys.RemoveOrg(id8);
  sys.RemoveOrg(id10);

  sys.AddSnapshotFun([](const emp::Taxon<int> & t){return std::to_string(t.GetInfo());}, "genome", "genome");
  sys.Snapshot("systematics_snapshot.csv");

  // TODO: Would be nice to compare this to existing snapshot file, but lines could be in any order
}

TEST_CASE("Test Prune", "[Evolve]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, false, false);

  int prunes = 0;
  std::function<void(emp::Ptr<emp::Taxon<int> >)> prune_fun = [&prunes](emp::Ptr<emp::Taxon<int>> tax){prunes++;};
  sys.OnPrune(prune_fun);

  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id5);
  sys.SetUpdate(6);
  auto id7 = sys.AddOrg(30, id1);
  sys.SetUpdate(33);
  auto id8 = sys.AddOrg(2, id3);
  auto id9 = sys.AddOrg(4, id8);
  sys.SetUpdate(34);
  auto id10 = sys.AddOrg(5, id9);
  auto id11 = sys.AddOrg(5, id3);

  sys.SetUpdate(40);
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);
  sys.RemoveOrg(id3);
  sys.RemoveOrg(id8);
  sys.RemoveOrg(id9);

  CHECK(sys.GetMRCA() == id1);

  CHECK(prunes == 0);
  CHECK(Has(sys.GetAncestors(), id9));
  sys.RemoveOrg(id10);
  CHECK(prunes == 3);
  CHECK(!Has(sys.GetAncestors(), id9));
  CHECK(Has(sys.GetAncestors(), id3));

  sys.RemoveOrg(id11);
  CHECK(prunes == 5);
  CHECK(!Has(sys.GetAncestors(), id3));
  CHECK(sys.GetMRCA() == id1);

  sys.RemoveOrg(id7);
  CHECK(prunes == 6);
  CHECK(sys.GetMRCA() == id2);
}

TEST_CASE("Test tracking position", "[Evolve]") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, true);

  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, {0,0}, nullptr);
  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, {1,0}, id1);
  CHECK(sys.Parent(id2) == id1);
  sys.SetNextParent(id1);
  sys.SetUpdate(10);
  sys.AddOrg(26, {2,0});
  auto id3 = sys.GetMostRecent();
  CHECK(id3->GetParent() == id1);
  CHECK(id3->GetInfo() == 26);
  CHECK(id3->GetOriginationTime() == 10);
  sys.SetNextParent({1,0});
  sys.SetUpdate(25);
  sys.AddOrg(27, {3,0});
  auto id4 = sys.GetMostRecent();
  CHECK(id4->GetParent() == id2);
  CHECK(id4->GetInfo() == 27);
  CHECK(id4->GetOriginationTime() == 25);

  sys.SetUpdate(40);
  sys.RemoveOrg({0,0});
  CHECK(id1->GetDestructionTime() == 40);
  CHECK(id1->GetNumOrgs() == 0);

  sys.RemoveOrgAfterRepro(id4);
  CHECK(!Has(sys.GetAncestors(), id4));
  sys.SetUpdate(34);
  auto id5 = sys.AddOrg(88, {4,0}, id4);
  CHECK(id4->GetNumOrgs() == 0);
  CHECK(id4->GetNumOff() == 1);
  CHECK(Has(sys.GetAncestors(), id4));
}

TEST_CASE("Test Total Offspring") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, false, false);

  auto org1 = sys.AddOrg(1, nullptr);
  auto org2 = sys.AddOrg(2, org1);
  auto org3 = sys.AddOrg(3, org2);
  auto org4 = sys.AddOrg(4, org3);
  auto org5 = sys.AddOrg(5, org3);
  auto org6 = sys.AddOrg(6, org2);
  auto org7 = sys.AddOrg(7, org6);
  auto org8 = sys.AddOrg(8, org6);
  auto org9 = sys.AddOrg(9, org1);
  auto org10 = sys.AddOrg(10, org9);
  auto org11 = sys.AddOrg(11, org9);

  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 10);

  CHECK(org2->GetNumOff() == 2);
  CHECK(org2->GetTotalOffspring() == 6);

  CHECK(org3->GetNumOff() == 2);
  CHECK(org3->GetTotalOffspring() == 2);

  CHECK(org4->GetNumOff() == 0);
  CHECK(org4->GetTotalOffspring() == 0);

  CHECK(org5->GetNumOff() == 0);
  CHECK(org5->GetTotalOffspring() == 0);

  CHECK(org6->GetNumOff() == 2);
  CHECK(org6->GetTotalOffspring() == 2);

  CHECK(org7->GetNumOff() == 0);
  CHECK(org7->GetTotalOffspring() == 0);

  CHECK(org8->GetNumOff() == 0);
  CHECK(org8->GetTotalOffspring() == 0);

  CHECK(org9->GetNumOff() == 2);
  CHECK(org9->GetTotalOffspring() == 2);

  CHECK(org10->GetNumOff() == 0);
  CHECK(org10->GetTotalOffspring() == 0);

  CHECK(org11->GetNumOff() == 0);
  CHECK(org11->GetTotalOffspring() == 0);

  sys.RemoveOrg(org1);

  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 10);

  sys.RemoveOrg(org2);

  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 9);

  CHECK(org2->GetNumOff() == 2);
  CHECK(org2->GetTotalOffspring() == 6);

  sys.RemoveOrg(org3);

  CHECK(org3->GetNumOff() == 2);
  CHECK(org3->GetTotalOffspring() == 2);
  CHECK(org2->GetNumOff() == 2);
  CHECK(org2->GetTotalOffspring() == 5);
  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 8);

  sys.RemoveOrg(org4);

  CHECK(org3->GetNumOff() == 1);
  CHECK(org3->GetTotalOffspring() == 1);
  CHECK(org2->GetNumOff() == 2);
  CHECK(org2->GetTotalOffspring() == 4);
  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 7);


  sys.RemoveOrg(org9);
  CHECK(org1->GetNumOff() == 2);
  CHECK(org1->GetTotalOffspring() == 6);

}

TEST_CASE("Test Degree Distribution") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, false, false);

  auto org1 = sys.AddOrg(1, nullptr);
  auto org2 = sys.AddOrg(2, org1);
  auto org3 = sys.AddOrg(3, org2);
  auto org4 = sys.AddOrg(4, org3);
  auto org5 = sys.AddOrg(5, org3);
  auto org6 = sys.AddOrg(6, org2);
  auto org7 = sys.AddOrg(7, org6);
  auto org8 = sys.AddOrg(8, org6);
  auto org9 = sys.AddOrg(9, org1);
  auto org10 = sys.AddOrg(10, org9);
  auto org11 = sys.AddOrg(11, org9);
  auto org12 = sys.AddOrg(12, org1);
  auto org13 = sys.AddOrg(13, org4);
  auto org14 = sys.AddOrg(14, org13);

  std::unordered_map<int, int> dist = sys.GetOutDegreeDistribution();
  CHECK(dist[0] == 7);
  CHECK(dist[1] == 2);
  CHECK(dist[2] == 4);
  CHECK(dist[3] == 1);

}

TEST_CASE("Test Average Origin Time") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, false);

  sys.SetUpdate(0);
  auto id1 = sys.AddOrg(25, nullptr);
  CHECK(sys.GetAverageOriginTime() == 0);
  CHECK(sys.GetAverageOriginTime(true) == 0);

  sys.SetUpdate(6);
  auto id2 = sys.AddOrg(-10, id1);
  CHECK(sys.GetAverageOriginTime() == 3);
  CHECK(sys.GetAverageOriginTime(true) == 0);

  sys.SetUpdate(10);
  auto id3 = sys.AddOrg(26, id1);
  CHECK(sys.GetAverageOriginTime() == Approx(5.333333));
  CHECK(sys.GetAverageOriginTime(true) == 0);

  sys.SetUpdate(25);
  auto id4 = sys.AddOrg(27, id2);
  CHECK(sys.GetAverageOriginTime() == Approx(10.25));
  CHECK(sys.GetAverageOriginTime(true) == Approx(0));

  sys.SetUpdate(32);
  auto id5 = sys.AddOrg(28, id2);
  CHECK(sys.GetAverageOriginTime() == Approx(14.6));
  CHECK(sys.GetAverageOriginTime(true) == Approx(3));

  sys.SetUpdate(39);
  auto id6 = sys.AddOrg(29, id2);
  CHECK(sys.GetAverageOriginTime() == Approx(18.6666667));
  CHECK(sys.GetAverageOriginTime(true) == Approx(4));

  CHECK(sys.CalcDiversity() == Approx(2.58496));

  sys.SetUpdate(39);
  auto id7 = sys.AddOrg(30, id2);
  CHECK(sys.GetAverageOriginTime() == Approx(21.571428571));
  CHECK(sys.GetAverageOriginTime(true) == Approx(4.5));
}

TEST_CASE("Test Loading Phylogeny From File") {
  emp::Systematics<int, int> sys([](const int & i){return i;}, true, true, true, true);
  sys.LoadFromFile("systematics_snapshot.csv", "genome");
  CHECK(sys.GetNumRoots() == 1);
  emp::Ptr<emp::Taxon<int>> mrca = sys.GetMRCA();
  CHECK(mrca->GetID() == 1);
  auto offspring = mrca->GetOffspring();
  for (auto off : offspring) {
    CHECK(((off->GetID() == 7) || (off->GetID() == 2) || (off->GetID() == 3)));
  }
  CHECK(sys.GetNumActive() == 6);
  CHECK(sys.GetNumAncestors() == 4);
  CHECK(sys.GetNumOutside() == 0);
  CHECK(sys.GetNumTaxa() == 10);
  CHECK(sys.GetMaxDepth() == 4);
  CHECK(mrca->GetTotalOffspring() == 6);
  CHECK(mrca->GetNumOff() == 3);
  CHECK(mrca->GetNumOrgs() == 0);

  for (auto tax : sys.GetActive()) {
    CHECK(((tax->GetNumOrgs() == 1) || (tax->GetID() == 10 )));
  }

  sys.PrintStatus();

  emp::Systematics<int, int> sys2([](const int & i){return i;}, true, true, true, true);
  sys2.LoadFromFile("systematics_snapshot.csv", "genome", false, false);
  CHECK(sys2.GetNumRoots() == 1);
  emp::Ptr<emp::Taxon<int>> mrca2 = sys.GetMRCA();
  CHECK(mrca2->GetID() == 1);
  auto offspring2 = mrca2->GetOffspring();
  for (auto off : offspring2) {
    CHECK(((off->GetID() == 7) || (off->GetID() == 2) || (off->GetID() == 3)));
  }
  CHECK(sys2.GetNumActive() == 5);
  CHECK(sys2.GetNumAncestors() == 5);
  CHECK(sys2.GetNumOutside() == 0);
  CHECK(sys2.GetNumTaxa() == 10);
  CHECK(sys2.GetMaxDepth() == 3);
  CHECK(mrca2->GetTotalOffspring() == 6);
  CHECK(mrca2->GetNumOff() == 3);
  CHECK(mrca2->GetNumOrgs() == 0);

  for (auto tax : sys2.GetActive()) {
    CHECK(tax->GetNumOrgs() == 1);
  }

  sys2.PrintStatus();
}

TEST_CASE("Test LoadFromFile and Snapshot behavior") {
  for (const auto& file : std::filesystem::directory_iterator("assets/")) {
    if (file.path().extension() == ".csv") {
      // load systematics from original file
      emp::Systematics<int, std::string> sys([](const int & i){ return emp::to_string(i); }, true, true, true, true);
      sys.LoadFromFile(file.path(), "phenotype");

      sys.AddSnapshotFun([](const emp::Systematics<int, std::string>::taxon_t& tax){ return emp::to_string(tax.GetInfo()); }, "phenotype");

      // save systematics into temp file
      const auto temp_path = std::filesystem::temp_directory_path() / file.path().filename();
      sys.Snapshot(temp_path);

      // load original systematics file
      emp::File original{file.path()};

      // load saved file
      emp::File saved{temp_path};

      CHECK(saved.AsSet() == original.AsSet());
    }
  }

}
