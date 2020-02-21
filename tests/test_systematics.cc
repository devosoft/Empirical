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
  emp::Systematics<int, int> sys([](int & i){return i;}, true, true, true);

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

  std::cout << "\nLineage:\n";
  sys.PrintLineage(id4);
  sys.PrintStatus();
}

TEST_CASE("Pointer to systematics", "[evo]") {
  emp::Ptr<emp::Systematics<int, int>> sys;
  sys.New([](int & i){return i;}, true, true, true);
  sys.Delete();
}

TEST_CASE("Test Data Struct", "[evo]")
{

  emp::Ptr<emp::Systematics<int, int, emp::datastruct::mut_landscape_info<int> >> sys;
  sys.New([](int & i){return i;}, true, true, true);
  auto id1 = sys->AddOrg(1, nullptr);
  id1->GetData().fitness.Add(2);
  id1->GetData().phenotype = 6;

  auto id2 = sys->AddOrg(2, id1);
  id2->GetData().mut_counts["substitution"] = 2;
  id2->GetData().fitness.Add(1);
  id2->GetData().phenotype = 6;
  REQUIRE(id2->GetData().mut_counts["substitution"] == 2);

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


  REQUIRE(CountMuts(id4) == 3);
  REQUIRE(CountDeleteriousSteps(id4) == 1);
  REQUIRE(CountPhenotypeChanges(id4) == 1);
  REQUIRE(CountUniquePhenotypes(id4) == 2);

  REQUIRE(CountMuts(id3) == 5);
  REQUIRE(CountDeleteriousSteps(id3) == 1);
  REQUIRE(CountPhenotypeChanges(id3) == 0);
  REQUIRE(CountUniquePhenotypes(id3) == 1);

  REQUIRE(CountMuts(id5) == 4);
  REQUIRE(CountDeleteriousSteps(id5) == 2);
  REQUIRE(CountPhenotypeChanges(id5) == 2);
  REQUIRE(CountUniquePhenotypes(id5) == 2);

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
  sys.New([](emp::vector<int> & v){return v;}, true, true, true);
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


  std::function<emp::AvidaGP::genome_t(emp::AvidaGP &)> gene_fun =
    [](emp::AvidaGP & org) {
      return org.GetGenome();
    };

  std::function<emp::vector<double>(emp::AvidaGP &)> phen_fun =
    [](emp::AvidaGP & org) {
      emp::vector<double> phen;
      for (int i = 0; i < 16; i++) {
        org.ResetHardware();
        org.Process(20);
        phen.push_back(org.GetOutput(i));
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
