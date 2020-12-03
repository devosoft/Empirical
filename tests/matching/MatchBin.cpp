#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "emp/data/DataNode.hpp"
#include "emp/matching/MatchBin.hpp"
#include "emp/math/Random.hpp"

#include <sstream>
#include <string>

TEST_CASE("Test MatchBin", "[matchbin]")
{

  // test baseline default N (1)
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin_rts(rand);

  bin_rts.Put("hi", 0);
  bin_rts.Put("salut", 0);
  REQUIRE(bin_rts.Match(0).size() == 1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::ExpRouletteSelector<>,
    emp::LegacyRegulator
  >bin_xrs(rand);

  bin_xrs.Put("hi", 0);
  bin_xrs.Put("salut", 0);
  REQUIRE(bin_xrs.Match(0).size() == 1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<>,
    emp::LegacyRegulator
  >bin_rks(rand);

  bin_rks.Put("hi", 0);
  bin_rks.Put("salut", 0);
  REQUIRE(bin_rks.Match(0).size() == 1);

  }

  // test setting different default N
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1000,1>,
      std::ratio<1, 1>,
      2
    >,
    emp::LegacyRegulator
  >bin_rts(rand);

  bin_rts.Put("hi", 0);
  bin_rts.Put("salut", 0);
  REQUIRE(bin_rts.Match(0).size() == 2);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::ExpRouletteSelector<
      std::ratio<13, 10>,
      std::ratio<1, 100>,
      std::ratio<4, 1>,
      std::ratio<4, 1>,
      std::ratio<5, 4>,
      2
    >,
    emp::LegacyRegulator
  >bin_xrs(rand);

  bin_xrs.Put("hi", 0);
  bin_xrs.Put("salut", 0);
  REQUIRE(bin_xrs.Match(0).size() == 2);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<-1,1>, 2>,
    emp::LegacyRegulator
  >bin_rks(rand);

  bin_rks.Put("hi", 0);
  bin_rks.Put("salut", 0);
  REQUIRE(bin_rks.Match(0).size() == 2);

  }

  {
    emp::Random rand(1);
  // We care about numbers less than 6 (<=5.99) away from what we're matching.
  // The AbsDiffMetric will normalize our result to distance / Max_Int.
  // We multiply both sides by 100 to get rid of floats for std::ratio
  // Add Max_INT * 100 in the numerator to offset the regulator function.
  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<214748364700+599,214748364700>, 2>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
  REQUIRE( bin.GetVal(bin.Put("yo", -4)) == "yo" );
  REQUIRE( bin.GetVal(bin.Put("konichiwa", -6)) == "konichiwa" );

  REQUIRE( bin.Size() == 5 );

  // std::numeric_limits<size_t>::max() = use Selector default n of 2
  REQUIRE(
    bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, std::numeric_limits<size_t>::max())) == (emp::vector<int>{0, 1}) );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 3)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == emp::vector<std::string>{} );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<int>{}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == emp::vector<std::string>{"bonjour"} );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<int>{6}) );

  bin.SetRegulator(hi, 0.1);
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"hi"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{1} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"hi", "salut"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{1, 0}) );

  bin.Delete(hi);
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, -4}) );

  bin.Put("hi", 1);
  // std::numeric_limits<size_t>::max() = use Selector default of 2
  REQUIRE(
    bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );


  }

  // test infinite thresh
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );
  const size_t bonjour = bin.Put("bonjour", std::numeric_limits<int>::max());
  REQUIRE(bin.GetVal(bonjour) == "bonjour");

  REQUIRE( bin.Size() == 3 );

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );

  bin.SetRegulator(bonjour, std::numeric_limits<double>::infinity());
  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()-1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 10.0);
  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 2.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  }


  //test roulette selector with threshold
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<(size_t)std::numeric_limits<int>::max()+1000000, std::numeric_limits<int>::max()>
    >,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", 1000000);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t yo = bin.Put("yo", std::numeric_limits<int>::max());
  REQUIRE( bin.GetVal(yo) == "yo" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 3 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 0.9);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(yo, 0.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") > 0 );

  bin.SetRegulator(salut, 20.0);
  bin.SetRegulator(hi, 20.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") == 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") == 0 );

  }

  // test RouletteSelector skew
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<std::ratio<-1,1>,std::ratio<1000,1>>,
    emp::LegacyRegulator
  >bin_softskew(rand);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<std::ratio<-1,1>,std::ratio<1,1000>>,
    emp::LegacyRegulator
  >bin_hardskew(rand);

  const size_t hi1 = bin_softskew.Put("hi", 100000000);
  REQUIRE( bin_softskew.GetVal(hi1) == "hi" );
  const size_t salut1 = bin_softskew.Put("salut", 0);
  REQUIRE( bin_softskew.GetVal(salut1) == "salut" );

  REQUIRE( bin_softskew.Size() == 2 );

  const size_t hi2 = bin_hardskew.Put("hi", 100000000);
  REQUIRE( bin_hardskew.GetVal(hi2) == "hi" );
  const size_t salut2 = bin_hardskew.Put("salut", 0);
  REQUIRE( bin_hardskew.GetVal(salut2) == "salut" );

  REQUIRE( bin_hardskew.Size() == 2 );


  auto res_softskew = bin_softskew.GetVals(bin_softskew.Match(0, 100000));
  const size_t count_soft = std::count(
    std::begin(res_softskew), std::end(res_softskew), "salut"
  );
  REQUIRE(count_soft > 45000);
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "hi") > 40000
  );

  auto res_hardskew = bin_hardskew.GetVals(bin_hardskew.Match(0, 100000));
  const size_t count_hard = std::count(
    std::begin(res_hardskew), std::end(res_hardskew), "salut"
  );
  REQUIRE( count_hard > 90000);
  REQUIRE( count_hard > count_soft);

  bin_softskew.AdjRegulator(salut1, 4.0);
  bin_softskew.SetRegulator(hi1, 0.5);
  res_softskew = bin_softskew.GetVals(bin_softskew.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "salut") > 45000
  );
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "hi") > 45000
  );

  bin_hardskew.AdjRegulator(salut2, 4.0);
  bin_hardskew.SetRegulator(hi2, 0.5);
  res_hardskew = bin_hardskew.GetVals(bin_hardskew.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_hardskew), std::end(res_hardskew), "hi") > 90000
  );

  }

  // test RouletteSelector MaxBaselineRatio
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1,10>,
      std::ratio<0,1>
    >,
    emp::LegacyRegulator
  >bin_lobase(rand);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1,10>,
      std::ratio<-1,1>
    >,
    emp::LegacyRegulator
  >bin_hibase(rand);

  const size_t hi1 = bin_lobase.Put("hi", std::numeric_limits<int>::max());
  REQUIRE( bin_lobase.GetVal(hi1) == "hi" );
  const size_t salut1 = bin_lobase.Put("salut", 0);
  REQUIRE( bin_lobase.GetVal(salut1) == "salut" );

  REQUIRE( bin_lobase.Size() == 2 );

  const size_t hi2 = bin_hibase.Put("hi", std::numeric_limits<int>::max());
  REQUIRE( bin_hibase.GetVal(hi2) == "hi" );
  const size_t salut2 = bin_hibase.Put("salut", 0);
  REQUIRE( bin_hibase.GetVal(salut2) == "salut" );

  REQUIRE( bin_hibase.Size() == 2 );

  auto res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  const size_t count_lobase = std::count(
    std::begin(res_lobase), std::end(res_lobase), "salut"
  );
  REQUIRE(count_lobase < 70000);
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 30000
  );

  auto res_hibase = bin_hibase.GetVals(bin_hibase.Match(0, 100000));
  const size_t count_hibase = std::count(
    std::begin(res_hibase), std::end(res_hibase), "salut"
  );
  REQUIRE( count_hibase > 90000);
  REQUIRE( count_hibase > count_lobase);

  bin_lobase.AdjRegulator(salut1, 9.0);
  bin_lobase.SetRegulator(hi1, 5.0);
  res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "salut") > 45000
  );
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 45000
  );

  bin_hibase.AdjRegulator(salut2, 4.0);
  bin_hibase.SetRegulator(hi2, 2.0);
  res_hibase = bin_hibase.GetVals(bin_hibase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_hibase), std::end(res_hibase), "hi") > 90000
  );

  bin_lobase.AdjRegulator(hi1, -5.0);
  bin_lobase.SetRegulator(salut1, 1.2);
  res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 90000
  );

  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::HammingMetric<32>,
    emp::RankedSelector<std::ratio<32 + 3, 32>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<32> bs3;
  bs3.SetUInt(0,3); // 0000 0011

  const size_t three = bitBin.Put("three", bs3);
  REQUIRE( bitBin.GetVal(three) == "three");

  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);
  REQUIRE( bitBin.GetVal(one) == "one");

  emp::BitSet<32> bs11;
  bs11.SetUInt(0,11); //0000 1011

  REQUIRE(bitBin.GetVal(bitBin.Put("eleven", bs11)) == "eleven");

  emp::BitSet<32> bs0;//0000 0000

  // rely on MatchBin default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs0)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs0)) == emp::vector<emp::BitSet<32>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs0, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs0, 1)) == emp::vector<emp::BitSet<32>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs11, 2)) == (emp::vector<std::string>{"eleven", "three"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs11, 2)) == (emp::vector<emp::BitSet<32>>{bs11, bs3}));

  {
  const auto res{ bitBin.GetVals(bitBin.Match(bs3, 5)) };

  const bool in_first_okay_order =
    res == (emp::vector<std::string> {"three","one","eleven"})
  ;
  const bool in_second_okay_order =
    res == (emp::vector<std::string> {"three","eleven","one"})
  ;
  REQUIRE( (in_first_okay_order || in_second_okay_order) );
  }

  {
  const auto res{ bitBin.GetTags(bitBin.Match(bs3, 5)) };

  const bool in_first_okay_order =
    res == (emp::vector<emp::BitSet<32>> {bs3, bs1, bs11})
  ;
  const bool in_second_okay_order =
    res == (emp::vector<emp::BitSet<32>> {bs3, bs11, bs1})
  ;
  REQUIRE( (in_first_okay_order || in_second_okay_order) );
  }

  REQUIRE (bitBin.Size() == 3);

  bitBin.SetRegulator(one, .1); //1 0 1 2 --> .2 .1 .2 .3

  REQUIRE(bitBin.GetVals(bitBin.Match(bs3, 5)) == (emp::vector<std::string> {"one","three","eleven"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs3, 5)) == (emp::vector<emp::BitSet<32>> {bs1, bs3, bs11}));

  bitBin.SetRegulator(one, 1);
  bitBin.SetRegulator(three, 4); // 2 1 0 1 --> 12 8 4 8

  REQUIRE(bitBin.GetVals(bitBin.Match(bs3, 5)) == (emp::vector<std::string> {"eleven", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs3, 5)) == (emp::vector<emp::BitSet<32>> {bs11, bs1}));

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::HammingMetric<32>,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bitBin(rand);

  emp::BitSet<32> bs2;
  bs2.SetUInt(0, 2);//0000 0010
  const size_t elementary = bitBin.Put("elementary", bs2);
  REQUIRE(bitBin.GetVal(elementary) == "elementary");

  emp::BitSet<32> bs6;
  bs6.SetUInt(0,6);//0000 0110
  const size_t fedora = bitBin.Put("fedora", bs6);
  REQUIRE	(bitBin.GetVal(fedora) == "fedora");

  REQUIRE(bitBin.Size() == 2);

  auto res = bitBin.GetVals(bitBin.Match(bs2, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "elementary");
  REQUIRE(count > 50000);
  REQUIRE (std::count(std::begin(res), std::end(res), "fedora") > 0);

  bitBin.AdjRegulator(elementary, 10);
  bitBin.SetRegulator(fedora, 0.5);
  res = bitBin.GetVals(bitBin.Match(bs2, 100000));

  REQUIRE(std::count(std::begin(res), std::end(res), "elementary") > 0);
  REQUIRE(std::count(std::begin(res), std::end(res), "fedora") > 50000);

  bitBin.SetRegulator(elementary, 0.5);
  bitBin.SetRegulator(fedora, 2.0);
  res = bitBin.GetVals(bitBin.Match(bs2, 100000));

  REQUIRE( std::count(std::begin(res), std::end(res), "elementary") > count);
  REQUIRE( std::count(std::begin(res), std::end(res), "fedora") >  0);

  }

  {
  emp::Random rand(1);
  const int max_value = 1000;
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<1000>,
    emp::RankedSelector<std::ratio<max_value + max_value,max_value>>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
  REQUIRE( bin.GetVal(bin.Put("yo", 10)) == "yo" );
  REQUIRE( bin.GetVal(bin.Put("konichiwa", max_value)) == "konichiwa" );

  REQUIRE( bin.Size() == 5 );

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<size_t>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<size_t>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<size_t>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(7, 3)) == (emp::vector<std::string>{"yo", "konichiwa", "salut"})
  );
  REQUIRE( bin.GetTags(bin.Match(7, 3)) == (emp::vector<size_t>{10, max_value, 0}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<size_t>{0, 1, 6, 10}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == (emp::vector<std::string>{"konichiwa", "salut", "hi", "bonjour", "yo"}) );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<size_t>{max_value, 0, 1, 6, 10}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == (emp::vector<std::string>{"yo", "konichiwa"}) );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<size_t>{10, max_value}) );

  bin.SetRegulator(hi, 1.004);
  REQUIRE( bin.GetVals(bin.Match(1, 1)) == emp::vector<std::string>{"hi"} );
  REQUIRE( bin.GetTags(bin.Match(1, 1)) == emp::vector<size_t>{1} );

  bin.SetRegulator(hi, 2.0);
  REQUIRE(
    bin.GetVals(bin.Match(1, 1)) == (emp::vector<std::string>{"bonjour"})
  );
  REQUIRE( bin.GetTags(bin.Match(1, 1)) == (emp::vector<size_t>{6}) );

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", 100000000);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 10.0);
  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 2.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::StreakMetric<8>,
    emp::RankedSelector<std::ratio<1+1, 1>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<8> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);
  REQUIRE( bitBin.GetVal(one) == "one");

  emp::BitSet<8> bs128;
  bs128.SetUInt(0,128); // 1000 000

  const size_t oneTwoEight = bitBin.Put("one-two-eight", bs128);
  REQUIRE( bitBin.GetVal(oneTwoEight) == "one-two-eight");

  emp::BitSet<8> bs127;
  bs127.SetUInt(0,127); //0111 1111

  REQUIRE(bitBin.GetVal(bitBin.Put("one-two-seven", bs127)) == "one-two-seven");

  emp::BitSet<8> bs15;
  bs15.SetUInt(0,15); //0000 1111

  REQUIRE(bitBin.GetVal(bitBin.Put("fifteen", bs15)) == "fifteen");

  emp::BitSet<8> bs2;//0000 0010
  bs2.SetUInt(0,2);

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, std::numeric_limits<size_t>::max())) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 1)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string>{"one-two-eight", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>>{bs128, bs1}));

  REQUIRE(bitBin.GetVals(bitBin.Match(bs127, 5)) == (emp::vector<std::string> {"one-two-seven", "fifteen","one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs127, 5)) == (emp::vector<emp::BitSet<8>> {bs127, bs15, bs1, bs128}));

  REQUIRE (bitBin.Size() == 4);

  bitBin.SetRegulator(one, .1); //1 0 1 2 --> .2 .1 .2 .3

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string> {"one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>> {bs1,bs128}));

  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::StreakMetric<64>,
    emp::RankedSelector<std::ratio<1+1, 1>>,
    emp::LegacyRegulator
  > bitBin64(rand);

  emp::BitSet<64> bs7;
  bs7.SetUInt(
    1,
    emp::IntPow(2UL, 18UL) + emp::IntPow(2UL, 19UL) + emp::IntPow(2UL, 20UL)
  );

  const size_t id_seven = bitBin64.Put("seven", bs7);
  REQUIRE( bitBin64.GetVal(id_seven) == "seven");

  emp::BitSet<64> bs1;
  bs1.SetUInt(
    1,
    emp::IntPow(2UL, 16UL) + emp::IntPow(2UL, 17UL) + emp::IntPow(2UL, 18UL)
  );

  const size_t id_one  = bitBin64.Put("one", bs1);
  REQUIRE( bitBin64.GetVal(id_one) == "one");

  emp::BitSet<64> bs9;
  bs9.SetUInt(
    1,
    emp::IntPow(2UL, 15UL) + emp::IntPow(2UL, 16UL) + emp::IntPow(2UL, 17UL)
  );

  const size_t id_nine  = bitBin64.Put("nine", bs9);
  REQUIRE( bitBin64.GetVal(id_nine) == "nine");

  REQUIRE (bitBin64.Size() == 3);

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE(bitBin64.GetVals(bitBin64.Match(bs9, std::numeric_limits<size_t>::max())) == (emp::vector<std::string> {"nine"}));
  REQUIRE(bitBin64.GetTags(bitBin64.Match(bs9, std::numeric_limits<size_t>::max())) == (emp::vector<emp::BitSet<64>> {bs9}));

  REQUIRE(bitBin64.GetVals(bitBin64.Match(bs9, 5)) == (emp::vector<std::string> {"nine","one","seven"}));
  REQUIRE(bitBin64.GetTags(bitBin64.Match(bs9, 5)) == (emp::vector<emp::BitSet<64>> {bs9, bs1, bs7}));

  }
  // test ImprintRegulators
  {
    // setup template MatchBin
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RouletteSelector<>,
      emp::LegacyRegulator
    > bin1(rand);

    // put some things in template
    const size_t hi1 = bin1.Put("hi", 1);
    const size_t bye1 = bin1.Put("bye", 2);

    bin1.SetRegulator(hi1, 0.1);
    bin1.SetRegulator(bye1, 0.2);

    // make sure regulators were set
    REQUIRE(bin1.GetRegulator(hi1).state == 0.1);
    REQUIRE(bin1.GetRegulator(bye1).state == 0.2);

    // setup MatchBin to imprint on template
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RouletteSelector<>,
      emp::LegacyRegulator
    > bin2(rand);

    // put same matches as before
    const size_t hi2 = bin2.Put("hi", 1);
    const size_t bye2 = bin2.Put("bye", 2);

    // do the imprinting
    bin2.ImprintRegulators(bin1);

    // now, bin2's regulators should match bin1's
    REQUIRE(bin2.GetRegulator(hi2).state == bin1.GetRegulator(hi1).state);
    REQUIRE(bin2.GetRegulator(bye2).state == bin1.GetRegulator(bye1).state);
  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::SymmetricNoWrapMetric<8>,
    emp::RankedSelector<std::ratio<256 + 40, 256>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<8> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);

  emp::BitSet<8> bs128;
  bs128.SetUInt(0,128); // 1000 000

  bitBin.Put("one-two-eight", bs128);

  emp::BitSet<8> bs127;
  bs127.SetUInt(0,127); //0111 1111

  bitBin.Put("one-two-seven", bs127);

  emp::BitSet<8> bs15;
  bs15.SetUInt(0,15); //0000 1111

  bitBin.Put("fifteen", bs15);

  emp::BitSet<8> bs2;//0000 0010
  bs2.SetUInt(0,2);

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, std::numeric_limits<size_t>::max())) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 1)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string>{"one-two-eight", "one-two-seven"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>>{bs128, bs127}));

  REQUIRE(bitBin.GetVals(bitBin.Match(bs127, 5)) == (emp::vector<std::string> {"one-two-seven", "one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs127, 5)) == (emp::vector<emp::BitSet<8>> {bs127, bs128}));

  REQUIRE (bitBin.Size() == 4);

  bitBin.SetRegulator(one, .001);

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string> {"one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>> {bs1,bs128}));

  bitBin.SetRegulator(one, 1.05);

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 5)) == (emp::vector<std::string> {"fifteen", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 5)) == (emp::vector<emp::BitSet<8>> {bs15, bs1}));

  }

// test HashMetric

  {

  const size_t N_SAMPLES = 1000000;
  const size_t N_BINS = 10;

  emp::HashMetric<32> metric;

  emp::BitSet<32> query;
  emp::BitSet<32> tag;

  emp::DataNode<double, emp::data::Range, emp::data::Histogram> scores;
  scores.SetupBins(0.0, 1.0, 10);

  emp::Random rand(2);

  // test with sequential numbers
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.SetUInt(0, i);
    tag.SetUInt(0, i);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(std::abs(scores.GetMean() - 0.5) < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.99);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with sequential numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.SetUInt(0, 0);
    tag.SetUInt(0, i);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(std::abs(scores.GetMean() - 0.5) < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.99);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with random numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.Randomize(rand);
    tag.Randomize(rand);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(std::abs(scores.GetMean() - 0.5) < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.99);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with weighted random numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.Randomize(rand, 0.75);
    tag.Randomize(rand, 0.75);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(std::abs(scores.GetMean() - 0.5) < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.99);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  }


  // test SlideMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::SlideMod<emp::StreakMetric<3>> slide_streak;
  emp::SlideMod<emp::HammingMetric<3>> slide_hamming;
  emp::StreakMetric<3> streak;

  REQUIRE(
    slide_streak(bs_100, bs_100)
    ==
    slide_streak(bs_100, bs_010)
  );

  REQUIRE(
    streak(bs_100, bs_100)
    ==
    slide_streak(bs_100, bs_010)
  );

  REQUIRE(
    slide_streak(bs_000, bs_111)
    >
    slide_streak(bs_100, bs_000)
  );

  REQUIRE(
    slide_streak(bs_000, bs_111)
    >
    streak(bs_100, bs_000)
  );

  REQUIRE(
    slide_hamming(bs_100, bs_100)
    ==
    slide_hamming(bs_100, bs_010)
  );
  }

  // test AntiMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_011{0,1,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::AntiMod<emp::StreakMetric<3>> anti_streak;
  emp::StreakMetric<3> streak;
  emp::AntiMod<emp::HammingMetric<3>> anti_hamming;
  emp::HammingMetric<3> hamming;


  // anti should be equivalent to 0 matching with 1 and vice versa
  // instead of 0 matching with 0 and 1 matching with 1
  REQUIRE(
    anti_streak(bs_000, bs_111)
    -
    streak(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_streak(bs_011, bs_000)
    -
    streak(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_hamming(bs_000, bs_111)
    -
    hamming(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_hamming(bs_011, bs_000)
    -
    hamming(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  }

  // text AntiMod x SlideMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_011{0,1,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::SlideMod<emp::StreakMetric<3>> slide_streak;
  emp::SlideMod<emp::AntiMod<emp::StreakMetric<3>>> slide_anti_streak;
  emp::SlideMod<emp::HammingMetric<3>> slide_hamming;
  emp::SlideMod<emp::AntiMod<emp::HammingMetric<3>>> slide_anti_hamming;

  // anti should be equivalent to 0 matching with 1 and vice versa
  // instead of 0 matching with 0 and 1 matching with 1

  REQUIRE(
    slide_streak(bs_000, bs_111)
    ==
    slide_anti_streak(bs_111, bs_111)
  );

  REQUIRE(
    slide_streak(bs_011, bs_000)
    ==
    slide_anti_streak(bs_011, bs_111)
  );

  REQUIRE(
    slide_hamming(bs_000, bs_111)
    -
    slide_anti_hamming(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    slide_hamming(bs_011, bs_000)
    -
    slide_anti_hamming(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );
  }

  // test MeanDimMod
  {
  emp::Random rand(1);

  emp::array<emp::BitSet<32>, 1> a1 = {
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 1> b1 = {
    emp::BitSet<32>(rand)
  };

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::MeanDimMod<emp::StreakMetric<32>, 1> d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::MeanDimMod<emp::AntiMod<emp::StreakMetric<32>>, 1> d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::MeanDimMod<emp::HammingMetric<32>, 1> d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::MeanDimMod<emp::SlideMod<emp::HammingMetric<32>>, 1> d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1[0], b1[0]) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1[0], b1[0]) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1[0], b1[0]) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1[0], b1[0]) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::MeanDimMod<emp::StreakMetric<32>, 3> d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::MeanDimMod<emp::AntiMod<emp::StreakMetric<32>>, 3> d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::MeanDimMod<emp::HammingMetric<32>, 3> d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::MeanDimMod<emp::SlideMod<emp::HammingMetric<32>>, 3> d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    streak(a3[0], b3[0]) + streak(a3[1], b3[1]) + streak(a3[2], b3[2])
    -
    d_streak3(a3,b3) * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    anti_streak(a3[0], b3[0]) + anti_streak(a3[1], b3[1]) + anti_streak(a3[2], b3[2])
    -
    d_anti_streak3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    hamming(a3[0], b3[0]) + hamming(a3[1], b3[1]) + hamming(a3[2], b3[2])
    -
    d_hamming3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    slide_hamming(a3[0], b3[0]) + slide_hamming(a3[1], b3[1]) + slide_hamming(a3[2], b3[2])
    -
    d_slide_hamming3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test MinDimMod
  {
  emp::Random rand(1);

  emp::array<emp::BitSet<32>, 1> a1 = {
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 1> b1 = {
    emp::BitSet<32>(rand)
  };

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::MinDimMod<emp::StreakMetric<32>, 1> d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::MinDimMod<emp::AntiMod<emp::StreakMetric<32>>, 1> d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::MinDimMod<emp::HammingMetric<32>, 1> d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::MinDimMod<emp::SlideMod<emp::HammingMetric<32>>, 1> d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1[0], b1[0]) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1[0], b1[0]) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1[0], b1[0]) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1[0], b1[0]) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::MinDimMod<emp::StreakMetric<32>, 3> d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::MinDimMod<emp::AntiMod<emp::StreakMetric<32>>, 3> d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::MinDimMod<emp::HammingMetric<32>, 3> d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::MinDimMod<emp::SlideMod<emp::HammingMetric<32>>, 3> d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    std::min({streak(a3[0], b3[0]), streak(a3[1], b3[1]), streak(a3[2], b3[2])})
    -
    d_streak3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({anti_streak(a3[0], b3[0]), anti_streak(a3[1], b3[1]), anti_streak(a3[2], b3[2])})
    -
    d_anti_streak3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({hamming(a3[0], b3[0]), hamming(a3[1], b3[1]), hamming(a3[2], b3[2])})
    -
    d_hamming3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({slide_hamming(a3[0], b3[0]), slide_hamming(a3[1], b3[1]), slide_hamming(a3[2], b3[2])})
    -
    d_slide_hamming3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test FlatMod x MeanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);

  emp::BitSet<32> b1(rand);

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      emp::StreakMetric<32>,
      1
    >
  > d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::FlatMod<
    emp::MeanDimMod<
      emp::AntiMod<
        typename emp::StreakMetric<32>
      >,
      1
    >
  > d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::FlatMod<
    emp::MeanDimMod<
      emp::SlideMod<
        typename emp::HammingMetric<32>
      >,
      1
    >
  > d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1, b1) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1, b1) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1, b1) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::BitSet<96> flat_a3;
  emp::BitSet<96> flat_b3;

  for (size_t i = 0; i < 96; ++i) {
    flat_a3[i] = a3[i/32][i%32];
    flat_b3[i] = b3[i/32][i%32];
  }

  emp::FlatMod<
    emp::MeanDimMod<
      emp::StreakMetric<32>,
      3
    >
  > d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::AntiMod<
        emp::StreakMetric<32>
      >,
      3
    >
  > d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::HammingMetric<32>,
      3
    >
  > d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::SlideMod<
        emp::HammingMetric<32>
      >,
      3
    >
  > d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    streak(a3[0], b3[0]) + streak(a3[1], b3[1]) + streak(a3[2], b3[2])
    -
    d_streak3(flat_a3,flat_b3) * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    anti_streak(a3[0], b3[0]) + anti_streak(a3[1], b3[1]) + anti_streak(a3[2], b3[2])
    -
    d_anti_streak3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    hamming(a3[0], b3[0]) + hamming(a3[1], b3[1]) + hamming(a3[2], b3[2])
    -
    d_hamming3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    slide_hamming(a3[0], b3[0]) + slide_hamming(a3[1], b3[1]) + slide_hamming(a3[2], b3[2])
    -
    d_slide_hamming3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test FlatMod x MinDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);

  emp::BitSet<32> b1(rand);

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::FlatMod<
    emp::MinDimMod<
      emp::StreakMetric<32>,
      1
    >
  > d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::FlatMod<
    emp::MinDimMod<
      emp::AntiMod<
        typename emp::StreakMetric<32>
      >,
      1
    >
  > d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::FlatMod<
    emp::MinDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::FlatMod<
    emp::MinDimMod<
      emp::SlideMod<
        typename emp::HammingMetric<32>
      >,
      1
    >
  > d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1, b1) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1, b1) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1, b1) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::BitSet<96> flat_a3;
  emp::BitSet<96> flat_b3;

  for (size_t i = 0; i < 96; ++i) {
    flat_a3[i] = a3[i/32][i%32];
    flat_b3[i] = b3[i/32][i%32];
  }

  emp::FlatMod<
    emp::MinDimMod<
      emp::StreakMetric<32>,
      3
    >
  > d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::AntiMod<
        emp::StreakMetric<32>
      >,
      3
    >
  > d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::HammingMetric<32>,
      3
    >
  > d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::SlideMod<
        emp::HammingMetric<32>
      >,
      3
    >
  > d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    std::min({streak(a3[0], b3[0]), streak(a3[1], b3[1]), streak(a3[2], b3[2])})
    -
    d_streak3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({anti_streak(a3[0], b3[0]), anti_streak(a3[1], b3[1]), anti_streak(a3[2], b3[2])})
    -
    d_anti_streak3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({hamming(a3[0], b3[0]), hamming(a3[1], b3[1]), hamming(a3[2], b3[2])})
    -
    d_hamming3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({slide_hamming(a3[0], b3[0]), slide_hamming(a3[1], b3[1]), slide_hamming(a3[2], b3[2])})
    -
    d_slide_hamming3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test SymmetricWrapMetric
  {
  const double norm = 8.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::SymmetricWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 7.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 5.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 5.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 1.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 7.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 6.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 2.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 2.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 1.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 4.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 3.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 7.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 4.0/norm);
  }

  // test SymmetricNoWrapMetric
  {
  const double norm = 15.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::SymmetricNoWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 7.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 11.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 15.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 7.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 10.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 14.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 1.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 4.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 3.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 7.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 4.0/norm);
  }

  // test AsymmetricWrapMetric
  {
  const double norm = 15.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::AsymmetricWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 15.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 9.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 5.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 10.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 9.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 2.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 15.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 12.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 13.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 9.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 12.0/norm);
  }

  // test AsymmetricNoWrapMetric
  {
  const double norm = 16.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::AsymmetricNoWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 16.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 16.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 16.0/norm);
  }
  {
  // Regulated Cache Testing
  struct DummySelector: public emp::RankedSelector<std::ratio<2,1>>{

    explicit DummySelector(emp::Random &rand) : emp::RankedSelector<std::ratio<2,1>>(rand) { ; }

    size_t opCount = 0;

    emp::RankedCacheState operator()(
      emp::vector< std::pair<size_t, double> > scores,
      size_t n
    ){
      opCount+=1;
      return emp::RankedSelector<std::ratio<2,1>>::operator()(
        scores,
        n
      );
    }
  };

  using parent_t = emp::MatchBin<
    emp::BitSet<32>,
    emp::HammingMetric<32>,
    DummySelector,
    emp::LegacyRegulator
  >;
  class MatchBinTest : public parent_t {
  public:
    explicit MatchBinTest(emp::Random & rand) : parent_t(rand) { ; }

    size_t GetSelectCount(){ return selector.opCount; }
  };

  emp::Random rand(1);
  MatchBinTest bin(rand);
  std::vector<size_t> ids;

  for(unsigned int i = 0; i < 1000; ++i){
    emp::BitSet<32> bs;
    bs.SetUInt32(0, i);
    ids.push_back(bin.Put(bs,bs));
  }

  REQUIRE( bin.GetRegulatedCacheSize() == 0);
  REQUIRE( bin.GetSelectCount() == 0);
  emp::vector<size_t> uncached = bin.Match(emp::BitSet<32>(), 10);// first match caches
  emp::vector<size_t> cached = bin.Match(emp::BitSet<32>(), 10);// second already cached
  REQUIRE( bin.GetRegulatedCacheSize() == 1);
  REQUIRE( bin.GetSelectCount() == 1);
  REQUIRE( cached == uncached );
  bin.DeactivateCaching();
  REQUIRE(bin.GetRegulatedCacheSize() == 0 );
  bin.Match(emp::BitSet<32>(),10);//second cache
  bin.Match(emp::BitSet<32>(),10);//third cache
  REQUIRE(bin.GetRegulatedCacheSize() == 0 );
  REQUIRE(bin.GetSelectCount() == 3);

  bin.ActivateCaching();
  REQUIRE(bin.GetRegulatedCacheSize() == 0 );


  for(unsigned int i = 0; i < 1000; ++i){
    emp::BitSet<32> bs;
    bs.SetUInt32(0, i);

    uncached = bin.Match(bs, 3);
    REQUIRE(bin.GetRegulatedCacheSize() == i + 1);
    REQUIRE(bin.GetSelectCount() == 3 + i + 1);

    cached = bin.Match(bs, 3);
    REQUIRE(bin.GetRegulatedCacheSize() == i + 1); //shouldnt change
    REQUIRE(bin.GetSelectCount() == 3 + i + 1); //shouldnt change

    REQUIRE(cached == uncached);
  }

  emp::BitSet<32> bs;
  bs.SetUInt32(0,1001);
  bin.SetTag(ids[0], bs);
  REQUIRE(bin.GetRegulatedCacheSize() == 0);

  bin.Match(emp::BitSet<32>(), 3);
  REQUIRE(bin.GetRegulatedCacheSize() == 1);
  REQUIRE(bin.GetSelectCount() == 1000 + 3 + 1);

  bin.Match(emp::BitSet<32>(), 4); //Asking for more than last time so we recache.
  REQUIRE(bin.GetRegulatedCacheSize() == 1); //replace the current one so same size.
  REQUIRE(bin.GetSelectCount() == 1000 + 3 + 2);
  }

  // Raw cache testing
  {
  struct DummySelector: public emp::RankedSelector<std::ratio<2,1>>{

    DummySelector(emp::Random &rand) : emp::RankedSelector<std::ratio<2,1>>(rand) { ; }

    size_t opCount = 0;
    emp::RankedCacheState operator()(
      emp::vector< std::pair<size_t, double> > scores,
      size_t n
    ){
      opCount+=1;
      return emp::RankedSelector<std::ratio<2,1>>::operator()(scores, n);
    }
  };

  using parent_t = emp::MatchBin<
    emp::BitSet<32>,
    emp::HammingMetric<32>,
    DummySelector,
    emp::LegacyRegulator
  >;
  class MatchBinTest : public parent_t {
  public:
    MatchBinTest(emp::Random & rand) : parent_t(rand) { ; }

    size_t GetSelectCount(){ return selector.opCount; }
  };

  emp::Random rand(1);
  MatchBinTest bin(rand);
  std::vector<size_t> ids;

  for(unsigned int i = 0; i < 1000; ++i){
    emp::BitSet<32> bs;
    bs.SetUInt32(0, i);
    ids.push_back(bin.Put(bs,bs));
  }
  // test raw caching
  REQUIRE( bin.GetRawCacheSize() == 0);
  REQUIRE( bin.GetSelectCount() == 0);
  emp::vector<size_t> uncached_raw = bin.MatchRaw(emp::BitSet<32>(), 10);// first match caches
  emp::vector<size_t> cached_raw = bin.MatchRaw(emp::BitSet<32>(), 10);// second already cached
  REQUIRE( bin.GetRawCacheSize() == 1);
  REQUIRE( bin.GetSelectCount() == 1);
  REQUIRE( uncached_raw == cached_raw );
  bin.DeactivateCaching();
  REQUIRE(bin.GetRawCacheSize() == 0 );
  bin.MatchRaw(emp::BitSet<32>(),10);//second cache
  bin.MatchRaw(emp::BitSet<32>(),10);//third cache
  REQUIRE(bin.GetRawCacheSize() == 0 );
  REQUIRE(bin.GetSelectCount() == 3);

  bin.ActivateCaching();
  REQUIRE(bin.GetRawCacheSize() == 0 );
  }
  // test clearing the cache
  {
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    > bin(rand);
    bin.ActivateCaching();

    // put some things in our matchbin
    bin.Put("1", 1);
    bin.Put("2", 2);

    // do some matches
    bin.Match(2);
    bin.MatchRaw(2);

    // these should be cached
    bin.Match(2);
    bin.MatchRaw(2);

    REQUIRE( bin.GetRegulatedCacheSize() == 1);
    REQUIRE( bin.GetRawCacheSize() == 1);

    // let's clear the cache!
    bin.ClearCache();

    // cache should be empty
    REQUIRE( bin.GetRegulatedCacheSize() == 0);
    REQUIRE( bin.GetRawCacheSize() == 0);

  }
  // serialization / deserialization
  {
  // set up
  emp::Random rand(1);
  std::stringstream ss;

  {
    // Create an output archive
    cereal::JSONOutputArchive oarchive(ss);

    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RouletteSelector<>,
      emp::LegacyRegulator
    > bin(rand);

    const size_t hi = bin.Put("hi", 1);
    REQUIRE( bin.GetVal(hi) == "hi" );
    const size_t salut = bin.Put("salut", 0);
    REQUIRE( bin.GetVal(salut) == "salut" );

    REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
    REQUIRE( bin.GetVal(bin.Put("yo", -4)) == "yo" );
    REQUIRE( bin.GetVal(bin.Put("konichiwa", -6)) == "konichiwa" );

    // Write the data to the archive
    oarchive(bin.GetState());

  } // archive goes out of scope, ensuring all contents are flushed

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<214748364700+599,214748364700>>,
    emp::LegacyRegulator
  > bin(rand);

  {
    cereal::JSONInputArchive iarchive(ss); // Create an input archive

    // Read the data from the archive
    decltype(bin)::state_t state;
    iarchive(state);
    bin.SetState(state);

  }

  REQUIRE( bin.Size() == 5 );

  // std::numeric_limits<size_t>::max() = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, std::numeric_limits<size_t>::max())) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 3)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == emp::vector<std::string>{} );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<int>{}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == emp::vector<std::string>{"bonjour"} );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<int>{6}) );


}

}
