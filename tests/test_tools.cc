//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the tools/ folder.


#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include <sstream>
#include <string>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>

#include "data/DataNode.h"

#include "tools/Binomial.h"
#include "tools/BitMatrix.h"
#include "tools/BitSet.h"
#include "tools/BitVector.h"
#include "tools/DFA.h"
#include "tools/DynamicString.h"
#include "tools/FunctionSet.h"
#include "tools/Graph.h"
#include "tools/IndexMap.h"
#include "tools/Lexer.h"
#include "tools/NFA.h"
#include "tools/RegEx.h"
#include "tools/Random.h"
#include "tools/TypeTracker.h"
#include "tools/attrs.h"

#include "tools/flex_function.h"
#include "tools/functions.h"
#include "tools/graph_utils.h"
#include "tools/hash_utils.h"
//#include "tools/grid.h"
#include "tools/info_theory.h"
#include "tools/lexer_utils.h"
#include "tools/map_utils.h"
#include "tools/math.h"
#include "tools/mem_track.h"
#include "tools/memo_function.h"
#include "tools/NullStream.h"
#include "tools/sequence_utils.h"
// #include "tools/serialize.h"
#include "tools/set_utils.h"
#include "tools/stats.h"
#include "tools/string_utils.h"
#include "tools/tuple_struct.h"
#include "tools/vector_utils.h"

// currently these have no coveage; we include them so we get metrics on them
// this doesn't actually work--TODO: figure out why this doesn't work
#include "tools/alert.h"
#include "tools/const.h"
#include "tools/SolveState.h"
#include "tools/serialize_macros.h"

/// Ensures that
/// 1) A == B
/// 2) A and B can be constexprs or non-contexprs.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_REQUIRE_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    REQUIRE(A == B);                     \
  }


TEST_CASE("Test Binomial", "[tools]")
{
  // test over a consistent set of seeds
  for (int s = 1; s <= 200; ++s) {

  REQUIRE(s > 0);
  emp::Random random(s);

  const double flip_prob = 0.03;
  const size_t num_flips = 100;

  const size_t num_tests = 100000;
  const size_t view_count = 10;

  emp::Binomial bi100(flip_prob, num_flips);

  emp::vector<size_t> counts(num_flips+1, 0);

  for (size_t test_id = 0; test_id < num_tests; test_id++) {
    size_t win_count = 0;
    for (size_t i = 0; i < num_flips; i++) {
      if (random.P(0.03)) win_count++;
    }
    counts[win_count]++;
  }

  // Print out the first values in the distribution.
  for (size_t i = 0; i < view_count; i++) {
    // std::cout << "bi100[" << i << "] = " << bi100[i]
    //           << "  test_count = " << counts[i]
    //           << "\n";
    REQUIRE(bi100[i] < ((double) counts[i]) / (double) num_tests + 0.02);
    REQUIRE(bi100[i] > ((double) counts[i]) / (double) num_tests - 0.02);
  }
  // std::cout << "Total = " << bi100.GetTotalProb() << std::endl;

  // // Pick some random values...
  // std::cout << "\nSome random values:";
  // for (size_t i = 0; i < 100; i++) {
  //   std::cout << " " << bi100.PickRandom(random);
  // }
  // std::cout << std::endl;
  }
}

// this templating is necessary to force full coverage of templated classes.
// Since c++ doesn't generate code for templated methods if those methods aren't
// explicitly called (and thus our profiling doesn't see them), we have to
// force them all to be included in the comilation.
template class emp::BitMatrix<4, 5>;
TEST_CASE("Test BitMatrix", "[tools]")
{

  emp::BitMatrix<4,5> bm45;

  REQUIRE(bm45.NumCols() == 4);
  REQUIRE(bm45.NumRows() == 5);
  REQUIRE(bm45.GetSize() == 20);

  REQUIRE(bm45.Any() == false);
  REQUIRE(bm45.None() == true);
  REQUIRE(bm45.All() == false);
  REQUIRE(bm45.Get(1,1) == 0);
  REQUIRE(bm45.Get(1,2) == 0);
  REQUIRE(bm45.CountOnes() == 0);

  bm45.Set(1,2);  // Try setting a single bit!

  REQUIRE(bm45.Any() == true);
  REQUIRE(bm45.None() == false);
  REQUIRE(bm45.All() == false);
  REQUIRE(bm45.Get(1,1) == 0);
  REQUIRE(bm45.Get(1,2) == 1);
  REQUIRE(bm45.CountOnes() == 1);
  REQUIRE(bm45.FindBit() == bm45.ToID(1,2));

  bm45.SetAll();
  REQUIRE(bm45.All() == true);
  REQUIRE(bm45.None() == false);
  bm45.ClearRow(2);
  REQUIRE(bm45.Get(2,2) == 0);
  REQUIRE(bm45.Get(2,1) == 1);
  bm45.ClearCol(1);
  REQUIRE(bm45.Get(1,1) == 0);
  bm45.Clear();
  REQUIRE(bm45.Get(0,2) == 0);
  bm45.SetRow(2);
  REQUIRE(bm45.Get(0,2) == 1);
  REQUIRE(bm45.Get(0,0) == 0);
  bm45.SetCol(0);
  REQUIRE(bm45.Get(0,0) == 1);
  bm45.Clear();
  bm45.SetRow(2);
  REQUIRE(bm45.Get(0,2) == 1);
  REQUIRE(bm45.Get(0,1) == 0);
  bm45.UpShift();
  // TODO: figure out how upshift actually works and write a real test for it



/* This block needs asserts
  bm45 = bm45.GetReach().GetReach();

  bm45 = bm45.DownShift();

  bm45 = bm45.RightShift();

  bm45 = bm45.URShift();

  bm45 = bm45.UpShift();

  bm45 = bm45.ULShift();

  bm45 = bm45.LeftShift();

  bm45 = bm45.DLShift();

  bm45 = bm45.DRShift();

  emp::BitMatrix<10,10> bm100;
  bm100.Set(9,9);
*/
}


template class emp::BitSet<5>;
TEST_CASE("Test BitSet", "[tools]")
{
  emp::BitSet<10> bs10;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  emp::BitSet<80> bs80c(bs80);
  bs80 <<= 1;

  for (size_t i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  REQUIRE(bs10[2] == false);
  bs10.flip(2);
  REQUIRE(bs10[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == false);}
  bs10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == true);}

  // Test importing....
  bs10.Import(bs80 >> 70);

  REQUIRE(bs10.GetUInt(0) == 2);

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUIntAtBit(64) == 130);
  REQUIRE(bs80.GetValueAtBit<5>(64) == 2);
}


TEST_CASE("Test BitSet timing", "[tools]")
{
  const size_t set_size = 100000;
  typedef emp::BitSet<set_size> TEST_TYPE;

  TEST_TYPE set1;
  TEST_TYPE set2;

  for (size_t i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  size_t total = 0;

  // should probably assert that this does what we want it to do...
  for (size_t i = 0; i < 100000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  double time = 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC;
  //REQUIRE(time < 13000); // WARNING: WILL VARY ON DIFFERENT SYSTEMS

  // END TIMING!!!
}


TEST_CASE("Test BitVector", "[tools]")
{
  emp::BitVector bv10(10);
  emp::BitVector bv32(32);
  emp::BitVector bv50(50);
  emp::BitVector bv64(64);
  emp::BitVector bv80(80);

  bv80[70] = 1;
  emp::BitVector bv80c(bv80);

  bv80 <<= 1;

  for (size_t i = 0; i < 75; i += 2) {
    emp::BitVector shift_vector = bv80 >> i;
    REQUIRE((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  bv10 = (bv80 >> 70);

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  REQUIRE(bv80.GetUIntAtBit(64) == 130);
  REQUIRE(bv80.GetValueAtBit<5>(64) == 2);
}


TEST_CASE("Test DFA", "[tools]")
{
  emp::DFA dfa(10);
  dfa.SetTransition(0, 1, 'a');
  dfa.SetTransition(1, 2, 'a');
  dfa.SetTransition(2, 0, 'a');
  dfa.SetTransition(0, 3, 'b');

  int state = 0;
  REQUIRE( (state = dfa.Next(state, 'a')) == 1 );
  REQUIRE( (state = dfa.Next(state, 'a')) == 2 );
  REQUIRE( (state = dfa.Next(state, 'a')) == 0 );
  REQUIRE( (state = dfa.Next(state, 'b')) == 3 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );

  REQUIRE(dfa.Next(0, "aaaaaab") == 3);
  REQUIRE(dfa.Next(0, "aaaaab") == -1);
  REQUIRE(dfa.Next(0, "aaaaaabb") == -1);
  REQUIRE(dfa.Next(0, "a") == 1);
  REQUIRE(dfa.Next(0, "aa") == 2);
  REQUIRE(dfa.Next(0, "aaa") == 0);
  REQUIRE(dfa.Next(0, "b")  == 3);
}

TEST_CASE("Test DynamicString", "[tools]")
{
  emp::DynamicString test_set;

  test_set.Append("Line Zero");  // Test regular append
  test_set << "Line One";        // Test stream append

  // Test append-to-stream
  std::stringstream ss;
  ss << test_set;
  REQUIRE(ss.str() == "Line ZeroLine One");

  // Test direct conversion to string.
  REQUIRE(test_set.str() == "Line ZeroLine One");

  // Test appending functions.
  test_set.Append( [](){ return std::string("Line Two"); } );
  test_set.Append( [](){ return "Line Three"; } );

  // Test appending functions with variable output
  int line_no = 20;
  test_set.Append( [&line_no](){ return std::string("Line ") + std::to_string(line_no); } );

  REQUIRE(test_set[4] == "Line 20");
  line_no = 4;
  REQUIRE(test_set[4] == "Line 4");

  // Make sure we can change an existing line.
  test_set.Set(0, "Line 0");
  REQUIRE(test_set[0] == "Line 0");

  // Make sure all lines are what we expect.
  REQUIRE(test_set[0] == "Line 0");
  REQUIRE(test_set[1] == "Line One");
  REQUIRE(test_set[2] == "Line Two");
  REQUIRE(test_set[3] == "Line Three");
  REQUIRE(test_set[4] == "Line 4");
}

int Sum4(int a1, int a2, int a3, int a4) {
  return a1 + a2 + a3 + a4;
}

TEST_CASE("Test flex_function", "[tools]")
{
  emp::flex_function<int(int,int,int,int)> ff = Sum4;
  ff.SetDefaults(10, 100,1000,10000);

  REQUIRE( ff(1,2,3,4) == 10 );
  REQUIRE( ff(1,2,3) == 10006 );
  REQUIRE( ff(1,2) == 11003 );
  REQUIRE( ff(1) == 11101 );
  REQUIRE( ff() == 11110 );
}

TEST_CASE("Test functions", "[tools]")
{

  bool test_bool = true;
  emp::Toggle(test_bool);
  REQUIRE(test_bool == false);

  REQUIRE(emp::ToRange(-10000, 10, 20) == 10);
  REQUIRE(emp::ToRange(9, 10, 20) == 10);
  REQUIRE(emp::ToRange(10, 10, 20) == 10);
  REQUIRE(emp::ToRange(11, 10, 20) == 11);
  REQUIRE(emp::ToRange(17, 10, 20) == 17);
  REQUIRE(emp::ToRange(20, 10, 20) == 20);
  REQUIRE(emp::ToRange(21, 10, 20) == 20);
  REQUIRE(emp::ToRange(12345678, 10, 20) == 20);
  REQUIRE(emp::ToRange<double>(12345678, 10, 20.1) == 20.1);
  REQUIRE(emp::ToRange(12345678.0, 10.7, 20.1) == 20.1);
}

// should migrate these inside the test case, probably

// utility funcs for FunctionSet test case
int global_var1;
int global_var2;
int global_var3;
int global_var4;

void fun1(int in1, int in2) { global_var1 = in1 + in2; }
void fun2(int in1, int in2) { global_var2 = in1 - in2; }
void fun3(int in1, int in2) { global_var3 = in1 * in2; }
void fun4(int in1, int in2) { global_var4 = in1 / in2; }

double fun5(double input) { return input; }
double fun6(double input) { return input * input; }
double fun7(double input) { return input * input * input; }

TEST_CASE("Test FunctionSet", "[tools]")
{
  // TEST 1: Functions with VOID returns.
  emp::FunctionSet<void(int,int)> fun_set;
  fun_set.Add(&fun1);
  fun_set.Add(&fun2);
  fun_set.Add(&fun3);
  fun_set.Add(&fun4);

  fun_set.Run(10, 2);

  REQUIRE(global_var1 == 12);
  REQUIRE(global_var2 == 8);
  REQUIRE(global_var3 == 20);
  REQUIRE(global_var4 == 5);

  fun_set.Remove(2);           // What if we remove a function

  fun_set.Run(20, 5);

  REQUIRE(global_var1 == 25);
  REQUIRE(global_var2 == 15);
  REQUIRE(global_var3 == 20);  // Same as last time!!
  REQUIRE(global_var4 == 4);

  // Test 2: Functions with non-void returns.
  emp::FunctionSet<double(double)> fun_set2;
  fun_set2.Add(&fun5);
  fun_set2.Add(&fun6);
  fun_set2.Add(&fun7);

  emp::vector<double> out_vals;

  out_vals = fun_set2.Run(10.0);

  REQUIRE(out_vals[0] == 10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == 1000.0);

  out_vals = fun_set2.Run(-10.0);

  REQUIRE(out_vals[0] == -10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == -1000.0);

  out_vals = fun_set2.Run(0.1);

  REQUIRE(out_vals[0] == 0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] < 0.00100001);
  REQUIRE(out_vals[2] > 0.00099999);

  out_vals = fun_set2.Run(-0.1);

  REQUIRE(out_vals[0] == -0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] > -0.00100001);
  REQUIRE(out_vals[2] < -0.00099999);


  // TEST 3: Running functions and auto-processing return values.
  REQUIRE(fun_set2.FindMax(10.0) == 1000.0);
  REQUIRE(fun_set2.FindMax(-10.0) == 100.0);
  REQUIRE(fun_set2.FindMax(0.1) == 0.1);
  REQUIRE(fun_set2.FindMax(-0.1) < 0.0100000001);
  REQUIRE(fun_set2.FindMax(-0.1) > 0.0099999999);

  REQUIRE(fun_set2.FindMin(10.0) == 10.0);
  REQUIRE(fun_set2.FindMin(-10.0) == -1000.0);
  REQUIRE(fun_set2.FindMin(0.1) < 0.0010000001);
  REQUIRE(fun_set2.FindMin(0.1) > 0.0009999999);
  REQUIRE(fun_set2.FindMin(-0.1) == -0.1);

  REQUIRE(fun_set2.FindSum(10.0) == 1110.0);
  REQUIRE(fun_set2.FindSum(-10.0) == -910.0);
  REQUIRE(fun_set2.FindSum(0.1) < 0.1110000001);
  REQUIRE(fun_set2.FindSum(0.1) > 0.1109999999);
  REQUIRE(fun_set2.FindSum(-0.1) == -0.091);


  // TEST 4: Manually-entered aggregation function.
  std::function<double(double,double)> test_fun =
    [](double i1, double i2){ return std::max(i1,i2); };

  REQUIRE(fun_set2.Run(10.0, test_fun) == 1000.0);
  REQUIRE(fun_set2.Run(-10.0, test_fun) == 100.0);
  REQUIRE(fun_set2.Run(0.1, test_fun) == 0.1);
  REQUIRE(fun_set2.Run(-0.1, test_fun) < 0.01000000001);
  REQUIRE(fun_set2.Run(-0.1, test_fun) > 0.00999999999);
}

// TODO: add moar asserts
TEST_CASE("Test graph", "[tools]")
{

  emp::Graph graph(20);

  REQUIRE(graph.GetSize() == 20);

  graph.AddEdgePair(0, 1);
  graph.AddEdgePair(0, 2);
  graph.AddEdgePair(0, 3);

}

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test Graph utils", "[tools]")
{
  emp::Random random(1);
  // emp::Graph graph( emp::build_graph_tree(20, random) );
  // emp::Graph graph( emp::build_graph_random(20, 40, random) );
  emp::Graph graph( emp::build_graph_grid(5, 4, random) );

  // graph.PrintSym();
}

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test hash_utils", "[tools]")
{

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);


  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);

  emp::vector<uint64_t> hash_vec;

  for(uint32_t i = 0; i < 10; ++i) {
    for(uint32_t j = 0; j < 10; ++j) {
      for(uint32_t s : { 0, 100, 100000 }) {
        hash_vec.push_back(emp::szudzik_hash(s+i,s+j));
      }
    }
  }

  std::unordered_set<uint64_t> hash_set(hash_vec.begin(), hash_vec.end());

  REQUIRE(hash_vec.size() == hash_set.size());

}


/*TEST_CASE("Test grid", "[tools]")
{
  emp::Grid::Layout layout(10,10);

  REQUIRE(layout.GetNumRegions() == 0);
  layout.AddRegion({0,1,2,3,4,5,6,7,8});

  REQUIRE(layout.GetNumRegions() == 1);

  emp::Grid::Board<void, void, void> board1(layout);
  emp::Grid::Board<bool, bool, bool> board2(layout);
  emp::Grid::Board<int, int, int> board3(layout);
  emp::Grid::Board<int, void, void> board4(layout);
  emp::Grid::Board<void, int, void> board5(layout);
  emp::Grid::Board<void, void, int> board6(layout);
  //TODO: moar asserts
}*/


// TODO: add moar asserts
TEST_CASE("Test IndexMap", "[tools]")
{
  emp::IndexMap imap(8);
  imap[0] = 1.0;
  imap[1] = 1.0;
  imap[2] = 1.0;
  imap[3] = 1.0;
  imap[4] = 2.0;
  imap[5] = 2.0;
  imap[6] = 0.0;
  imap[7] = 8.0;
  
  REQUIRE(imap.GetSize() == 8);
  REQUIRE(imap.GetWeight() == 16.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(4) == 0.125);
  REQUIRE(imap.GetProb(7) == 0.5);
  REQUIRE(imap.Index(7.1) == 5);

  // Add a new element to the end of the map that takes up half of the weight.
  imap.PushBack(16.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 32.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap.GetWeight(8) == 16.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(7) == 0.25);
  REQUIRE(imap.Index(7.1) == 5);
  REQUIRE(imap.Index(17.1) == 8);

  emp::IndexMap imap_bak(imap);
  imap.AdjustAll(10.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);

  // Did the backup copy work correctly?
  REQUIRE(imap_bak.GetSize() == 9);
  REQUIRE(imap_bak.GetWeight() == 32.0);
  REQUIRE(imap_bak.GetWeight(2) == 1.0);
  REQUIRE(imap_bak.GetWeight(5) == 2.0);
  REQUIRE(imap_bak.GetWeight(7) == 8.0);
  REQUIRE(imap_bak.GetWeight(8) == 16.0);
  REQUIRE(imap_bak[5] == 2.0);
  REQUIRE(imap_bak.GetProb(7) == 0.25);
  REQUIRE(imap_bak.Index(7.1) == 5);
  REQUIRE(imap_bak.Index(17.1) == 8);

  // Can we add on values from one index map to another?
  imap += imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 122.0);
  REQUIRE(imap.GetWeight(2) == 11.0);
  REQUIRE(imap.GetWeight(5) == 12.0);
  REQUIRE(imap.GetWeight(7) == 18.0);
  REQUIRE(imap.GetWeight(8) == 26.0);
  REQUIRE(imap[5] == 12.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(90.0) == 7);

  // And subtraction?
  imap -= imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);
}

TEST_CASE("Test info_theory", "[tools]")
{
  emp::vector<int> weights = { 100, 100, 200 };
  REQUIRE( emp::Entropy(weights) == 1.5 );

  emp::vector<double> dweights = { 10.5, 10.5, 10.5, 10.5, 21.0, 21.0 };
  REQUIRE( emp::Entropy(dweights) == 2.5 );

  REQUIRE( emp::Entropy2(0.5) == 1.0 );
}

TEST_CASE("Test lexer_utils", "[tools]")
{
  emp::NFA nfa2c(3);  // Must have zero or two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.AddFreeTransition(0,2);
  nfa2c.SetStop(2);

  emp::RegEx re2f("[de]*f[de]*f[de]*");
  // emp::RegEx re2f("([de]*)f([de]*)f([de]*)");
emp::NFA nfa2f = to_NFA(re2f);
emp::DFA dfa2f = to_DFA(nfa2f);
REQUIRE( nfa2f.GetSize() == 12 );
REQUIRE( dfa2f.GetSize() == 3 );

  int state;
  state = dfa2f.Next(0, "a");        REQUIRE(state == -1); REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "d");        REQUIRE(state == 0); REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defdef");   REQUIRE(state == 2); REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "fedfed");   REQUIRE(state == 2); REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "ffed");     REQUIRE(state == 2); REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffed");   REQUIRE(state == 2); REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffedf");  REQUIRE(state == -1); REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defed");    REQUIRE(state == 1); REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "ff");       REQUIRE(state == 2); REQUIRE(dfa2f.IsStop(state) == true);

  emp::RegEx re_lower("[a-z]+");
  emp::RegEx re_upper("[A-Z]+");
  emp::RegEx re_inc("[a-z]+[A-Z]+");
  emp::NFA nfa_lower = to_NFA(re_lower);
  emp::NFA nfa_upper = to_NFA(re_upper);
  emp::NFA nfa_inc = to_NFA(re_inc);
  emp::NFA nfa_all = MergeNFA(nfa_lower, nfa_upper, nfa_inc);
  emp::DFA dfa_lower = to_DFA(nfa_lower);
  emp::DFA dfa_upper = to_DFA(nfa_upper);
  emp::DFA dfa_inc = to_DFA(nfa_inc);
  emp::DFA dfa_all = to_DFA(nfa_all);

  emp::NFA_State lstate(nfa_lower);
  lstate.Reset(); lstate.Next("abc");      REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("DEF");      REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABDdef");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABCDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcdefghijklmnopqrstuvwxyz");  REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("ABC-DEF");  REQUIRE(lstate.IsActive() == false);

  REQUIRE( dfa_all.Next(0, "abc") == 2 );
  REQUIRE( dfa_all.Next(0, "DEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcDEF") == 3 );
  REQUIRE( dfa_all.Next(0, "ABDdef") == -1 );
  REQUIRE( dfa_all.Next(0, "ABCDEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcdefghijklmnopqrstuvwxyz") == 2 );
  REQUIRE( dfa_all.Next(0, "ABC-DEF") == -1 );
}


TEST_CASE("Test Lexer", "[tools]")
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.AddToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";

  REQUIRE(lexer.Process(ss).lexeme == "This");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "is");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "a");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "123");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "TEST");
  REQUIRE(lexer.Process(ss).lexeme == ".");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == " ");

  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Mixed");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Whitespace");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Lower");
}


TEST_CASE("Test map_utils", "[tools]")
{
  std::map<int, char> test_map;
  test_map[0] = 'a';
  test_map[4] = 'e';
  test_map[8] = 'i';
  test_map[14] = 'o';
  test_map[20] = 'u';

  REQUIRE( emp::Has(test_map, 8) == true );
  REQUIRE( emp::Has(test_map, 18) == false );
  REQUIRE( emp::Find(test_map, 14, 'x') == 'o'); // 14 should be there as 'o'
  REQUIRE( emp::Find(test_map, 15, 'x') == 'x'); // 15 shouldn't be there, so return default.
  REQUIRE( emp::Has(test_map, 15) == false );    // Make sure 15 hasn't been added to the map.

  auto flipped = emp::flip_map(test_map);        // Make sure we can reverse the map.
  REQUIRE( emp::Has(flipped, 'u') == true);      // And the reversed map should have proper info.
  REQUIRE( emp::Has(flipped, 'x') == false);

  // Testing for bug #123
  std::map<std::string, std::string> test_123;
  test_123["1"] = "1";
  test_123["12"] = "12";

  REQUIRE( emp::Find(test_123, "0", "nothing") == "nothing" );
  REQUIRE( emp::Find(test_123, "1", "nothing") == "1" );
  REQUIRE( emp::FindRef(test_123, "1", "nothing") == "1" );
}

TEST_CASE("Test math", "[tools]")
{
  constexpr auto a1 = emp::Log2(3.14);           REQUIRE( a1 > 1.650);   REQUIRE( a1 < 1.651);
  constexpr auto a2 = emp::Log2(0.125);          REQUIRE( a2 == -3.0 );
  constexpr auto a3 = emp::Log(1000, 10);        REQUIRE( a3 == 3.0 );
  constexpr auto a4 = emp::Log(10, 1000);        REQUIRE( a4 > 0.333 );  REQUIRE( a4 < 0.334 );
  constexpr auto a5 = emp::Log10(100);           REQUIRE( a5 == 2.0 );
  constexpr auto a6 = emp::Ln(3.33);             REQUIRE( a6 > 1.202 );  REQUIRE( a6 < 1.204 );
  constexpr auto a7 = emp::Pow2(2.345);          REQUIRE( a7 > 5.080 );  REQUIRE( a7 < 5.081 );
  constexpr auto a8 = emp::Pow(emp::PI, emp::E); REQUIRE( a8 > 22.440 ); REQUIRE( a8 < 22.441 );
  constexpr auto a9 = emp::Pow(7, 10);           REQUIRE( a9 == 282475249 );

  REQUIRE(emp::Mod(10, 7) == 3);
  REQUIRE(emp::Mod(3, 7) == 3);
  REQUIRE(emp::Mod(-4, 7) == 3);
  REQUIRE(emp::Mod(-11, 7) == 3);

  REQUIRE(emp::Mod(-11, 11) == 0);
  REQUIRE(emp::Mod(0, 11) == 0);
  REQUIRE(emp::Mod(11, 11) == 0);

  REQUIRE(emp::Pow(2,3) == 8);
  REQUIRE(emp::Pow(-2,2) == 4);
  REQUIRE(emp::IntPow(3,4) == 81);

  REQUIRE(emp::Min(5) == 5);
  REQUIRE(emp::Min(5,10) == 5);
  REQUIRE(emp::Min(10,5) == 5);
  REQUIRE(emp::Min(40,30,20,10,5,15,25,35) == 5);

  REQUIRE(emp::Max(5) == 5);
  REQUIRE(emp::Max(5,10) == 10);
  REQUIRE(emp::Max(10,5) == 10);
  REQUIRE(emp::Max(40,30,20,10,45,15,25,35) == 45);

  REQUIRE(emp::FloorDivide(0,4) == 0);
  REQUIRE(emp::FloorDivide(1,4) == 0);
  REQUIRE(emp::FloorDivide(2,4) == 0);
  REQUIRE(emp::FloorDivide(3,4) == 0);
  REQUIRE(emp::FloorDivide(4,4) == 1);
  REQUIRE(emp::FloorDivide(6,4) == 1);
  REQUIRE(emp::FloorDivide(5,3) == 1);
  REQUIRE(emp::FloorDivide(6,3) == 2);
  REQUIRE(emp::FloorDivide(7,3) == 2);

  REQUIRE(emp::FloorDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)2,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)3,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)5,(size_t)3) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::FloorDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::FloorDivide(-1,4) == -1);
  REQUIRE(emp::FloorDivide(-2,4) == -1);
  REQUIRE(emp::FloorDivide(-3,4) == -1);
  REQUIRE(emp::FloorDivide(-4,4) == -1);
  REQUIRE(emp::FloorDivide(-6,4) == -2);
  REQUIRE(emp::FloorDivide(-5,3) == -2);
  REQUIRE(emp::FloorDivide(-6,3) == -2);
  REQUIRE(emp::FloorDivide(-7,3) == -3);

  REQUIRE(emp::FloorDivide(0,-4) == 0);
  REQUIRE(emp::FloorDivide(1,-4) == -1);
  REQUIRE(emp::FloorDivide(2,-4) == -1);
  REQUIRE(emp::FloorDivide(3,-4) == -1);
  REQUIRE(emp::FloorDivide(4,-4) == -1);
  REQUIRE(emp::FloorDivide(6,-4) == -2);
  REQUIRE(emp::FloorDivide(5,-3) == -2);
  REQUIRE(emp::FloorDivide(6,-3) == -2);
  REQUIRE(emp::FloorDivide(7,-3) == -3);

  REQUIRE(emp::FloorDivide(-1,-4) == 0);
  REQUIRE(emp::FloorDivide(-2,-4) == 0);
  REQUIRE(emp::FloorDivide(-3,-4) == 0);
  REQUIRE(emp::FloorDivide(-4,-4) == 1);
  REQUIRE(emp::FloorDivide(-6,-4) == 1);
  REQUIRE(emp::FloorDivide(-5,-3) == 1);
  REQUIRE(emp::FloorDivide(-6,-3) == 2);
  REQUIRE(emp::FloorDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide(0,4) == 0);
  REQUIRE(emp::RoundedDivide(1,4) == 0);
  REQUIRE(emp::RoundedDivide(2,4) == 1);
  REQUIRE(emp::RoundedDivide(3,4) == 1);
  REQUIRE(emp::RoundedDivide(4,4) == 1);
  REQUIRE(emp::RoundedDivide(6,4) == 2);
  REQUIRE(emp::RoundedDivide(5,3) == 2);
  REQUIRE(emp::RoundedDivide(6,3) == 2);
  REQUIRE(emp::RoundedDivide(7,3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::RoundedDivide(-1,4) == 0);
  REQUIRE(emp::RoundedDivide(-2,4) == 0);
  REQUIRE(emp::RoundedDivide(-3,4) == -1);
  REQUIRE(emp::RoundedDivide(-4,4) == -1);
  REQUIRE(emp::RoundedDivide(-6,4) == -1);
  REQUIRE(emp::RoundedDivide(-5,3) == -2);
  REQUIRE(emp::RoundedDivide(-6,3) == -2);
  REQUIRE(emp::RoundedDivide(-7,3) == -2);

  REQUIRE(emp::RoundedDivide(0,-4) == 0);
  REQUIRE(emp::RoundedDivide(1,-4) == 0);
  REQUIRE(emp::RoundedDivide(2,-4) == 0);
  REQUIRE(emp::RoundedDivide(3,-4) == -1);
  REQUIRE(emp::RoundedDivide(4,-4) == -1);
  REQUIRE(emp::RoundedDivide(6,-4) == -1);
  REQUIRE(emp::RoundedDivide(5,-3) == -2);
  REQUIRE(emp::RoundedDivide(6,-3) == -2);
  REQUIRE(emp::RoundedDivide(7,-3) == -2);

  REQUIRE(emp::RoundedDivide(-1,-4) == 0);
  REQUIRE(emp::RoundedDivide(-2,-4) == 1);
  REQUIRE(emp::RoundedDivide(-3,-4) == 1);
  REQUIRE(emp::RoundedDivide(-4,-4) == 1);
  REQUIRE(emp::RoundedDivide(-6,-4) == 2);
  REQUIRE(emp::RoundedDivide(-5,-3) == 2);
  REQUIRE(emp::RoundedDivide(-6,-3) == 2);
  REQUIRE(emp::RoundedDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  auto MeanUnbiasedDivide = [](int dividend, int divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(MeanUnbiasedDivide(0,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(3,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(4,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(5,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(6,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(7,3,100) == 2);

  REQUIRE(MeanUnbiasedDivide(-1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(-3,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-4,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(-5,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-6,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-7,3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(0,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(3,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(4,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(5,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(6,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(7,-3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(-1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(-3,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-4,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(-5,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-6,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-7,-3,100) == 2);

  auto SztMeanUnbiasedDivide = [](size_t dividend, size_t divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(SztMeanUnbiasedDivide((size_t)0,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)1,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) > 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) < 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)3,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)4,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) > 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) < 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)5,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)7,(size_t)3,100) == 2);

  REQUIRE(emp::Sgn(1) == 1);
  REQUIRE(emp::Sgn(2) == 1);
  REQUIRE(emp::Sgn(3) == 1);
  REQUIRE(emp::Sgn(102) == 1);
  REQUIRE(emp::Sgn(0) == 0);
  REQUIRE(emp::Sgn(-1) == -1);
  REQUIRE(emp::Sgn(-2) == -1);
  REQUIRE(emp::Sgn(-3) == -1);
  REQUIRE(emp::Sgn(-102) == -1);

  REQUIRE(emp::Sgn((size_t)1) == 1);
  REQUIRE(emp::Sgn((size_t)2) == 1);
  REQUIRE(emp::Sgn((size_t)3) == 1);
  REQUIRE(emp::Sgn((size_t)102) == 1);
  REQUIRE(emp::Sgn((size_t)0) == 0);

  REQUIRE(emp::Sgn(1.0) == 1);
  REQUIRE(emp::Sgn(2.1) == 1);
  REQUIRE(emp::Sgn(3.0) == 1);
  REQUIRE(emp::Sgn(102.5) == 1);
  REQUIRE(emp::Sgn(0.0) == 0);
  REQUIRE(emp::Sgn(-1.0) == -1);
  REQUIRE(emp::Sgn(-2.1) == -1);
  REQUIRE(emp::Sgn(-3.0) == -1);
  REQUIRE(emp::Sgn(-102.5) == -1);

}


struct TestClass1 {
  TestClass1() {
    EMP_TRACK_CONSTRUCT(TestClass1);
  }
  ~TestClass1() {
    EMP_TRACK_DESTRUCT(TestClass1);
  }
};

struct TestClass2 {
  TestClass2() {
    EMP_TRACK_CONSTRUCT(TestClass2);
  }
  ~TestClass2() {
    EMP_TRACK_DESTRUCT(TestClass2);
  }
};

TEST_CASE("Test mem_track", "[tools]")
{
  emp::vector<TestClass1 *> test_v;
  TestClass2 class2_mem;

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 0);
  #endif

  for (int i = 0; i < 1000; i++) {
    test_v.push_back( new TestClass1 );
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 1000);
  #endif

  for (size_t i = 500; i < 1000; i++) {
    delete test_v[i];
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 500);
  //REQUIRE(EMP_TRACK_STATUS == 0);
  #endif

}


TEST_CASE("Test emp::memo_function", "[tools]")
{
  emp::memo_function<uint64_t(int)> test_fun;

  // Build a Fibonacchi function...
  test_fun = [&test_fun](int N) {
    if (N<=1) return (uint64_t) N;
    return test_fun(N-1) + test_fun(N-2);
  };

  REQUIRE( test_fun(80) == 0x533163ef0321e5 );
}


TEST_CASE("Test NFA", "[tools]")
{
  emp::NFA nfa(10);
  nfa.AddTransition(0, 1, 'a');
  nfa.AddTransition(0, 2, 'a');
  nfa.AddTransition(0, 3, 'a');
  nfa.AddTransition(0, 4, 'a');

  nfa.AddTransition(1, 2, 'b');
  nfa.AddTransition(2, 3, 'c');
  nfa.AddTransition(3, 4, 'd');

  nfa.AddTransition(0, 1, 'e');
  nfa.AddTransition(0, 1, 'f');
  nfa.AddTransition(0, 1, 'g');

  nfa.AddTransition(2, 3, 'a');
  nfa.AddTransition(3, 4, 'a');
  nfa.AddTransition(2, 4, 'a');

  nfa.AddTransition(2, 2, 'e');
  nfa.AddTransition(3, 3, 'e');
  nfa.AddTransition(4, 4, 'e');

  nfa.AddFreeTransition(1,5);

  nfa.AddTransition(5, 6, 'a');

  nfa.AddFreeTransition(6,7);
  nfa.AddFreeTransition(6,8);
  nfa.AddFreeTransition(6,9);
  nfa.AddFreeTransition(9,0);

  emp::NFA_State state(nfa);
  REQUIRE(state.GetSize() == 1);
  state.Next('a');
  REQUIRE(state.GetSize() == 5);
  state.Next('a');
  REQUIRE(state.GetSize() == 7);

  emp::NFA_State state2(nfa);
  REQUIRE(state2.GetSize() == 1);
  state2.Next("aaaa");
  REQUIRE(state2.GetSize() == 7);
}


TEST_CASE("Test NullStream", "[tools]")
{
  emp::NullStream ns;
  ns << "abcdefg";
  ns << std::endl;
  ns << 123;
  ns << 123.456;
  ns.flush();
}


TEST_CASE("Test random", "[tools]")
{
  // test over a consistent set of seeds
  for(int s = 1; s < 102; ++s) {

  REQUIRE(s > 0);
  emp::Random rng(s);

  // Test GetDouble with the law of large numbers.
  emp::vector<int> val_counts(10);
  for (size_t i = 0; i < val_counts.size(); i++) val_counts[i] = 0;

  const size_t num_tests = 100000;
  const double min_value = 2.5;
  const double max_value = 8.7;
  double total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const double cur_value = rng.GetDouble(min_value, max_value);
    total += cur_value;
    val_counts[(size_t) cur_value]++;
  }

  {
    const double expected_mean = (min_value + max_value) / 2.0;
    const double min_threshold = (expected_mean*0.995);
    const double max_threshold = (expected_mean*1.005);
    double mean_value = total/(double) num_tests;

    REQUIRE(mean_value > min_threshold);
    REQUIRE(mean_value < max_threshold);
  }

  // Test GetInt
  for (size_t i = 0; i < val_counts.size(); i++) val_counts[i] = 0;
  total = 0.0;

  for (size_t i = 0; i < num_tests; i++) {
    const size_t cur_value = rng.GetUInt(min_value, max_value);
    total += cur_value;
    val_counts[cur_value]++;
  }

  {
    const double expected_mean = (double) (((int) min_value) + ((int) max_value) - 1) / 2.0;
    const double min_threshold = (expected_mean*0.995);
    const double max_threshold = (expected_mean*1.005);
    double mean_value = total/(double) num_tests;

    REQUIRE(mean_value > min_threshold);
    REQUIRE(mean_value < max_threshold);
  }

  // Test GetUInt()
  emp::vector<uint32_t> uint32_draws;
  total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const uint32_t cur_value = rng.GetUInt();
    total += cur_value;
    uint32_draws.push_back(cur_value);
  }

  {
  const double expected_mean = ((double)std::numeric_limits<uint32_t>::max())/2.0;
  const double min_threshold = (expected_mean*0.995);
  const double max_threshold = (expected_mean*1.005);
  double mean_value = total/(double) num_tests;

  REQUIRE(mean_value > min_threshold);
  REQUIRE(mean_value < max_threshold);
  // ensure that all bits are set at least once and unset at least once
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(uint32_draws.begin(),uint32_draws.end(),(uint32_t)0,
    [](uint32_t accumulator, uint32_t val){ return accumulator | val; })
  );
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(uint32_draws.begin(),uint32_draws.end(),(uint32_t)0,
    [](uint32_t accumulator, uint32_t val){ return accumulator | (~val); })
  );
  }
  // Test GetUInt64
  emp::vector<uint64_t> uint64_draws;
  total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const uint64_t cur_value = rng.GetUInt64();
    total += cur_value/(double)num_tests;
    uint64_draws.push_back(cur_value);
  }

  {
  const double expected_mean = ((double)std::numeric_limits<uint64_t>::max())/2.0;
  const double min_threshold = (expected_mean*0.995);
  const double max_threshold = (expected_mean*1.005);
  double mean_value = total; // values were divided by num_tests when added

  REQUIRE(mean_value > min_threshold);
  REQUIRE(mean_value < max_threshold);
  // ensure that all bits are set at least once and unset at least once
  REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(uint64_draws.begin(),uint64_draws.end(),(uint64_t)0,
    [](uint64_t accumulator, uint64_t val){ return accumulator | val; })
  );
  REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(uint64_draws.begin(),uint64_draws.end(),(uint64_t)0,
    [](uint64_t accumulator, uint64_t val){ return accumulator | (~val); })
  );
  }

  // Test P
  double flip_prob = 0.56789;
  int hit_count = 0;
  for (size_t i = 0; i < num_tests; i++) {
    if (rng.P(flip_prob)) hit_count++;
  }

  double actual_prob = ((double) hit_count) / (double) num_tests;

  REQUIRE(actual_prob < flip_prob + 0.01);
  REQUIRE(actual_prob > flip_prob - 0.01);


  // Mimimal test of Choose()
  emp::vector<size_t> choices = Choose(rng,100,10);

  REQUIRE(choices.size() == 10);
  }
}


TEST_CASE("Test regular expressions (RegEx)", "[tools]")
{
  emp::RegEx re1("a|bcdef");
  REQUIRE(re1.Test("a") == true);
  REQUIRE(re1.Test("bc") == false);
  REQUIRE(re1.Test("bcdef") == true);
  REQUIRE(re1.Test("bcdefg") == false);

  emp::RegEx re2("#[abcdefghijklm]*abc");
  REQUIRE(re2.Test("") == false);
  REQUIRE(re2.Test("#a") == false);
  REQUIRE(re2.Test("#aaaabc") == true);
  REQUIRE(re2.Test("#abcabc") == true);
  REQUIRE(re2.Test("#abcabcd") == false);

  emp::RegEx re3("xx(y|(z*)?)+xx");
  REQUIRE(re3.Test("xxxx") == true);
  REQUIRE(re3.Test("xxxxx") == false);
  REQUIRE(re3.Test("xxyxx") == true);
  REQUIRE(re3.Test("xxyyxx") == true);
  REQUIRE(re3.Test("xxzzzxx") == true);

  emp::RegEx re_WHITESPACE("[ \t\r]");
  emp::RegEx re_COMMENT("#.*");
  emp::RegEx re_INT_LIT("[0-9]+");
  emp::RegEx re_FLOAT_LIT("[0-9]+[.][0-9]+");
  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  emp::RegEx re_STRING_LIT("[\"]((\\\\[nt\"\\\\])|[^\"])*\\\"");
  emp::RegEx re_ID("[a-zA-Z0-9_]+");

  REQUIRE(re_INT_LIT.Test("1234") == true);
  REQUIRE(re_FLOAT_LIT.Test("1234") == false);
  REQUIRE(re_ID.Test("1234") == true);
  REQUIRE(re_INT_LIT.Test("1234.56") == false);
  REQUIRE(re_FLOAT_LIT.Test("1234.56") == true);
  REQUIRE(re_ID.Test("1234.56") == false);

  std::string test_str = "\"1234\"";
  REQUIRE(re_STRING_LIT.Test(test_str) == true);
  REQUIRE(re_INT_LIT.Test(test_str) == false);

  std::string test_str2 = "\"1234\", \"5678\"";
  REQUIRE(re_STRING_LIT.Test(test_str2) == false);
}

TEST_CASE("Test sequence utils", "[tools]")
{
  std::string s1 = "This is the first test string.";
  std::string s2 = "This is the second test string.";

  REQUIRE(emp::calc_hamming_distance(s1,s2) == 19);
  REQUIRE(emp::calc_edit_distance(s1,s2) == 6);

  // std::string s3 = "abcdefghijklmnopqrstuvwWxyz";
  // std::string s4 = "abBcdefghijXXmnopqrstuvwxyz";

  // std::string s3 = "lmnopqrstuv";
  // std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  std::string s3 = "adhlmnopqrstuvxy";
  std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  emp::align(s3, s4, '_');

  REQUIRE(s3 == "a__d___h___lmnopqrstuv_xy_");

  emp::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  emp::vector<int> v2 = { 1,4,5,6,8 };

  emp::align(v1,v2,0);

  REQUIRE((v2 == emp::vector<int>({1,0,0,4,5,6,0,8,0})));
}



// struct SerializeTest {
//   int a;
//   float b;        // unimportant data!
//   std::string c;

//   SerializeTest(int _a, float _b, std::string _c) : a(_a), b(_b), c(_c) { ; }
//   EMP_SETUP_DATAPOD(SerializeTest, a, c);
// };

// struct SerializeTest_D : public SerializeTest {
//   char d = '$';

//   SerializeTest_D(int _a, float _b, std::string _c, char _d)
//     : SerializeTest(_a, _b, _c), d(_d) { ; }
//   EMP_SETUP_DATAPOD_D(SerializeTest_D, SerializeTest, d);
// };

// struct ExtraBase {
//   double e;

//   ExtraBase(double _e) : e(_e) { ; }
//   EMP_SETUP_DATAPOD(ExtraBase, e);
// };

// struct MultiTest : public SerializeTest, public ExtraBase {
//   bool f;

//   MultiTest(int _a, float _b, std::string _c, double _e, bool _f)
//     : SerializeTest(_a, _b, _c), ExtraBase(_e), f(_f) { ; }
//   EMP_SETUP_DATAPOD_D2(MultiTest, SerializeTest, ExtraBase, f);
// };

// struct NestedTest {
//   SerializeTest st;
//   std::string name;
//   SerializeTest_D std;
//   MultiTest mt;

//   NestedTest(int a1, float b1, std::string c1,
//              int a2, float b2, std::string c2, char d2,
//              int a3, float b3, std::string c3, double e3, bool f3)
//     : st(a1, b1, c1), name("my_class"), std(a2, b2, c2, d2), mt(a3, b3, c3, e3, f3) { ; }

//   EMP_SETUP_DATAPOD(NestedTest, st, name, std, mt);
// };

// struct BuiltInTypesTest {
//   const int a;
//   emp::vector<int> int_v;

//   BuiltInTypesTest(int _a, size_t v_size) : a(_a), int_v(v_size) {
//     for (size_t i = 0; i < v_size; i++) int_v[i] = (int)(i*i);
//   }

//   EMP_SETUP_DATAPOD(BuiltInTypesTest, a, int_v);
// };

// TEST_CASE("Test serialize", "[tools]")
// {
//   std::stringstream ss;
//   emp::serialize::DataPod pod(ss);


//   // Basic test...

//   SerializeTest st(7, 2.34, "my_test_string");
//   st.EMP_Store(pod);

//   SerializeTest st2(pod);

//   REQUIRE(st2.a == 7);                 // Make sure a was reloaded correctly.
//   REQUIRE(st2.c == "my_test_string");  // Make sure c was reloaded correctly.


//   // Derived class Test

//   SerializeTest_D stD(10,0.2,"three",'D');
//   stD.EMP_Store(pod);

//   SerializeTest_D stD2(pod);

//   REQUIRE(stD2.a == 10);
//   REQUIRE(stD2.c == "three");
//   REQUIRE(stD2.d == 'D');

//   // Multiply-derived class Test

//   MultiTest stM(111,2.22,"ttt",4.5,true);
//   stM.EMP_Store(pod);

//   MultiTest stM2(pod);


//   REQUIRE(stM2.a == 111);
//   REQUIRE(stM2.c == "ttt");
//   REQUIRE(stM2.e == 4.5);
//   REQUIRE(stM2.f == true);


//   // Nested objects test...

//   NestedTest nt(91, 3.14, "magic numbers",
//                 100, 0.01, "powers of 10", '1',
//                 1001, 1.001, "ones and zeros", 0.125, true);
//   nt.EMP_Store(pod);

//   NestedTest nt2(pod);

//   REQUIRE(nt2.st.a == 91);
//   REQUIRE(nt2.st.c == "magic numbers");
//   REQUIRE(nt2.name == "my_class");
//   REQUIRE(nt2.std.a == 100);
//   REQUIRE(nt2.std.c == "powers of 10");
//   REQUIRE(nt2.std.d == '1');
//   REQUIRE(nt2.mt.a == 1001);
//   REQUIRE(nt2.mt.c == "ones and zeros");
//   REQUIRE(nt2.mt.e == 0.125);
//   REQUIRE(nt2.mt.f == true);


//   // If we made it this far, everything must have worked!;

//   const int v_size = 43;
//   BuiltInTypesTest bitt(91, v_size);
//   bitt.EMP_Store(pod);


//   BuiltInTypesTest bitt2(pod);
// }



TEST_CASE("Test string_utils", "[tools]")
{

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  std::string escaped_string = emp::to_escaped_string(special_string);

  // note: we had to double-escape the test to make sure this worked.
  REQUIRE(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // TEST2: Test more general conversion to literals.
  REQUIRE(emp::to_literal(42) == "42");
  REQUIRE(emp::to_literal('a') == "'a'");
  REQUIRE(emp::to_literal('\t') == "'\\t'");
  REQUIRE(emp::to_literal(1.234) == "1.234000");

  // TEST3: Make sure that we can properly identify different types of characters.
  int num_ws = 0;
  int num_cap = 0;
  int num_lower = 0;
  int num_let = 0;
  int num_num = 0;
  int num_alphanum = 0;
  int num_i = 0;
  int num_vowel = 0;
  for (char cur_char : special_string) {
    if (emp::is_whitespace(cur_char)) num_ws++;
    if (emp::is_upper_letter(cur_char)) num_cap++;
    if (emp::is_lower_letter(cur_char)) num_lower++;
    if (emp::is_letter(cur_char)) num_let++;
    if (emp::is_digit(cur_char)) num_num++;
    if (emp::is_alphanumeric(cur_char)) num_alphanum++;
    if (emp::is_valid(cur_char, [](char c){ return c=='i'; })) num_i++;
    if (emp::is_valid(cur_char, [](char c){return c=='a' || c=='A';},
                      [](char c){return c=='e' || c=='E';},
                      [](char c){return c=='i' || c=='I';},
                      [](char c){return c=='o' || c=='O';},
                      [](char c){return c=='u' || c=='U';},
                      [](char c){return c=='y';}
                      )) num_vowel++;
  }
  int num_other = ((int) special_string.size()) - num_alphanum - num_ws;


  REQUIRE(num_ws == 6);
  REQUIRE(num_cap == 6);
  REQUIRE(num_lower == 16);
  REQUIRE(num_let == 22);
  REQUIRE(num_num == 3);
  REQUIRE(num_alphanum == 25);
  REQUIRE(num_other == 5);
  REQUIRE(num_i == 3);
  REQUIRE(num_vowel == 7);

  std::string base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best string!!!!   ";

  std::string first_line = emp::string_pop_line(base_string);

  REQUIRE(first_line == "This is an okay string.");
  REQUIRE(emp::string_get_word(first_line) == "This");

  emp::string_pop_word(first_line);

  REQUIRE(first_line == "is an okay string.");

  emp::remove_whitespace(first_line);

  REQUIRE(first_line == "isanokaystring.");

  std::string popped_str = emp::string_pop(first_line, "ns");

  REQUIRE(popped_str == "i");
  REQUIRE(first_line == "anokaystring.");


  popped_str = emp::string_pop(first_line, "ns");


  REQUIRE(popped_str == "a");
  REQUIRE(first_line == "okaystring.");


  popped_str = emp::string_pop(first_line, 'y');

  REQUIRE(popped_str == "oka");
  REQUIRE(first_line == "string.");

  emp::left_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!   ");

  emp::right_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!");

  emp::compress_whitespace(base_string);
  REQUIRE(base_string == "This is -MY- very best string!!!!");


  std::string view_test = "This is my view test!";
  REQUIRE( emp::view_string(view_test) == "This is my view test!" );
  REQUIRE( emp::view_string(view_test, 5) == "is my view test!" );
  REQUIRE( emp::view_string(view_test, 8, 2) == "my" );
  REQUIRE( emp::view_string_front(view_test,4) == "This" );
  REQUIRE( emp::view_string_back(view_test, 5) == "test!" );
  REQUIRE( emp::view_string_range(view_test, 11, 15) == "view" );
  REQUIRE( emp::view_string_to(view_test, ' ') == "This" );
  REQUIRE( emp::view_string_to(view_test, ' ', 5) == "is" );

  emp::vector<std::string_view> slice_view = emp::view_slices(view_test, ' ');
  REQUIRE( slice_view.size() == 5 );
  REQUIRE( slice_view[0] == "This" );
  REQUIRE( slice_view[1] == "is" );
  REQUIRE( slice_view[2] == "my" );
  REQUIRE( slice_view[3] == "view" );
  REQUIRE( slice_view[4] == "test!" );


  auto slices = emp::slice("This is a test of a different version of slice.", ' ');

  REQUIRE(slices.size() == 10);
  REQUIRE(slices[8] == "of");

  // Try other ways of using slice().
  emp::slice(base_string, slices, 's');

  REQUIRE(slices.size() == 5);
  REQUIRE(slices[1] == " i");
  REQUIRE(slices[3] == "t ");


  // Some tests of to_string() function.
  REQUIRE(emp::to_string((int) 1) == "1");
  REQUIRE(emp::to_string("2") == "2");
  REQUIRE(emp::to_string(std::string("3")) == "3");
  REQUIRE(emp::to_string('4') == "4");
  REQUIRE(emp::to_string((int16_t) 5) == "5");
  REQUIRE(emp::to_string((int32_t) 6) == "6");
  REQUIRE(emp::to_string((int64_t) 7) == "7");
  REQUIRE(emp::to_string((uint16_t) 8) == "8");
  REQUIRE(emp::to_string((uint32_t) 9) == "9");
  REQUIRE(emp::to_string((uint64_t) 10) == "10");
  REQUIRE(emp::to_string((size_t) 11) == "11");
  REQUIRE(emp::to_string((long) 12) == "12");
  REQUIRE(emp::to_string((unsigned long) 13) == "13");
  REQUIRE(emp::to_string((float) 14.0) == "14.000000");
  REQUIRE(emp::to_string((double) 15.0) == "15.000000");
  REQUIRE(emp::to_string(16.0) == "16.000000");
  REQUIRE(emp::to_string(emp::vector<size_t>({17,18,19})) == "[ 17 18 19 ]");
  REQUIRE(emp::to_string((char) 32) == " ");
  REQUIRE(emp::to_string((unsigned char) 33) == "!");

  std::string cat_a = "ABC";
  bool cat_b = true;
  char cat_c = '2';
  int cat_d = 3;

  std::string cat_full = emp::to_string(cat_a, cat_b, cat_c, cat_d);

  REQUIRE(cat_full == "ABC123");
  std::array<int, 3> test_arr({{ 4, 2, 5 }});
  REQUIRE(emp::to_string(test_arr) == "[ 4 2 5 ]");
}



TEST_CASE("Test stats", "[tools]") {
  emp::vector<int> vec1({1,2,1,1,2,3});
  double i1 = 1;
  double i2 = 1;
  double i3 = 1;
  double i4 = 2;

  emp::vector<double*> vec2({&i1, &i2, &i3, &i4});

  std::deque<double> deque1({5,4,3,5,4,6});

  REQUIRE(emp::ShannonEntropy(vec1) == Approx(1.459324));
  REQUIRE(emp::ShannonEntropy(vec2) == Approx(0.81128));
  REQUIRE(emp::ShannonEntropy(deque1) == Approx(1.918648));

  REQUIRE(emp::Variance(vec1) == Approx(0.55539));
  REQUIRE(emp::Variance(vec2) == Approx(0.1875));
  REQUIRE(emp::Variance(deque1) == Approx(0.9166666667));

  REQUIRE(emp::StandardDeviation(vec1) == Approx(0.745245));
  REQUIRE(emp::StandardDeviation(vec2) == Approx(0.433013));
  REQUIRE(emp::StandardDeviation(deque1) == Approx(0.957427));

  REQUIRE(emp::Sum(vec1) == 10);
  REQUIRE(emp::Sum(vec2) == 5);
  REQUIRE(emp::Sum(deque1) == 27);

  REQUIRE(emp::UniqueCount(vec1) == 3);
  REQUIRE(emp::UniqueCount(vec2) == 2);
  REQUIRE(emp::UniqueCount(deque1) == 4);

  REQUIRE(emp::Mean(vec1) == Approx(1.6666666666667));
  REQUIRE(emp::Mean(vec2) == Approx(1.25));
  REQUIRE(emp::Mean(deque1) == 4.5);

  std::function<int(int)> invert = [](int i){return i*-1;};

  REQUIRE(emp::MaxResult(invert, vec1) == -1);
  REQUIRE(emp::MinResult(invert, vec1) == -3);
  REQUIRE(emp::MeanResult(invert, vec1) == Approx(-1.666666667));
  REQUIRE(emp::ApplyFunction(invert, vec1) == emp::vector<int>({-1,-2,-1,-1,-2,-3}));

}

TEST_CASE("Test set utils", "[tools]") {
  std::set<int> s1;
  std::set<int> s2;
  std::set<int> comp_set;
  emp::vector<int> v1;
  emp::vector<int> v2;

  s1.insert(1);
  s1.insert(2);
  s2.insert(2);
  s2.insert(3);
  v1.push_back(1);
  v1.push_back(3);
  v2.push_back(4);
  v2.push_back(1);

  REQUIRE(emp::Has(s1, 1));
  REQUIRE(!emp::Has(s1, 3));

  comp_set.insert(1);
  REQUIRE(emp::difference(s1, s2) == comp_set);
  comp_set.clear();
  comp_set.insert(3);
  REQUIRE(emp::difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::intersection(s1, s2) == comp_set);
  REQUIRE(emp::intersection(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::difference(s1, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  REQUIRE(emp::intersection(s1, v1) == comp_set);
  REQUIRE(emp::intersection(v1, s1) == comp_set);
  REQUIRE(emp::intersection(v2, v1) == comp_set);
  REQUIRE(emp::intersection(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  REQUIRE(emp::difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::set_union(s1, s2) == comp_set);
  REQUIRE(emp::set_union(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, s2) == comp_set);
  REQUIRE(emp::set_union(s2, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, v1) == comp_set);
  REQUIRE(emp::set_union(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(s1, s2) == comp_set);
  REQUIRE(emp::symmetric_difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, v2) == comp_set);
  REQUIRE(emp::symmetric_difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, s1) == comp_set);
  REQUIRE(emp::symmetric_difference(s1, v1) == comp_set);

}

std::string tt_result;

// Some functions to print a single type and its value
void fun_int(int x) { tt_result = emp::to_string("int:", x); }
void fun_double(double x) { tt_result = emp::to_string("double:", x); }
void fun_string(std::string x) { tt_result = emp::to_string("string:", x); }

// And some silly ways to combine types.
void fun_int_int(int x, int y) { tt_result = emp::to_string(x+y); }
void fun_int_double(int x, double y) { tt_result = emp::to_string(y * (double) x); }
void fun_string_int(std::string x, int y) {
  tt_result = "";
  for (int i=0; i < y; i++) tt_result += x;
}
void fun_5ints(int v, int w, int x, int y, int z) {
  tt_result = emp::to_string(v, '+', w, '+', x, '+', y, '+', z, '=', v+w+x+y+z);
}

TEST_CASE("Test TypeTracker", "[tools]") {
  using tt_t = emp::TypeTracker<int, std::string, double>;   // Setup the tracker type.
  tt_t tt;                                                   // Build the tracker.

  // Add some functions.
  tt.AddFunction( [](int x){ tt_result = emp::to_string("int:", x); } );
  tt.AddFunction(fun_double);
  tt.AddFunction(fun_string);
  tt.AddFunction(fun_int_int);
  tt.AddFunction(fun_int_double);
  tt.AddFunction(fun_string_int);
  tt.AddFunction(fun_5ints);

  emp::TrackedVar tt_int1 = tt.Convert<int>(1);
  emp::TrackedVar tt_int2 = tt.Convert<int>(2);
  emp::TrackedVar tt_int3 = tt.Convert<int>(3);

  emp::TrackedVar tt_str  = tt.Convert<std::string>("FOUR");
  emp::TrackedVar tt_doub = tt.Convert<double>(5.5);

  tt.RunFunction(tt_int1, tt_int2);  // An int and another int should add.
  REQUIRE( tt_result == "3" );

  tt.RunFunction(tt_int3, tt_doub);  // An int and a double should multiply.
  REQUIRE( tt_result == "16.500000" );

  tt.RunFunction(tt_doub, tt_int2); // A double and an int is unknown; should leave old result.
  REQUIRE( tt_result == "16.500000" );

  tt.RunFunction(tt_str, tt_int3);    // A string an an int should duplicate the string.
  REQUIRE( tt_result == "FOURFOURFOUR" );

  tt.RunFunction(tt_int1, tt_int2, tt_int3, tt_int2, tt_int1);  // Add five ints!
  REQUIRE( tt_result == "1+2+3+2+1=9" );


  // Load all types into a vector and then experiment with them.
  emp::vector<emp::TrackedVar> vars;
  vars.push_back(tt_int1);
  vars.push_back(tt_int2);
  vars.push_back(tt_int3);
  vars.push_back(tt_str);
  vars.push_back(tt_doub);

  emp::vector<std::string> results = { "int:1", "int:2", "int:3", "string:FOUR", "double:5.5" };

  for (size_t i = 0; i < vars.size(); i++) {
    tt(vars[i]);
    REQUIRE(tt_result == results[i]);
  }

  // Make sure TypeTracker can determine consistant IDs.
  REQUIRE( (tt_t::GetID<int,std::string,double>()) == (tt_t::GetTrackedID(tt_int1, tt_str, tt_doub)) );
  REQUIRE( (tt_t::GetComboID<int,std::string,double>()) == (tt_t::GetTrackedComboID(tt_int1, tt_str, tt_doub)) );

  // Make sure a TypeTracker can work with a single type.
  size_t num_args = 0;
  emp::TypeTracker<int> tt1;
  tt1.AddFunction( [&num_args](int){ num_args=1; } );
  tt1.AddFunction( [&num_args](int,int){ num_args=2; } );
  tt1.AddFunction( [&num_args](int,int,int){ num_args=3; } );

  tt_int1 = tt1.Convert<int>(1);
  tt_int2 = tt1.Convert<int>(2);
  tt_int3 = tt1.Convert<int>(3);

  tt1.RunFunction(tt_int1);
  REQUIRE(num_args == 1);
  tt1(tt_int2, tt_int3);
  REQUIRE(num_args == 2);
  tt1(tt_int1, tt_int2, tt_int3);
  REQUIRE(num_args == 3);

}

TEST_CASE("Test vector utils", "[tools]") {
  emp::vector<int> v1({6,2,5,1,3});
  emp::Sort(v1);
  REQUIRE(v1 == emp::vector<int>({1,2,3,5,6}));
  REQUIRE(emp::FindValue(v1, 3) == 2);
  REQUIRE(emp::Sum(v1) == 17);
  REQUIRE(emp::Has(v1, 3));
  REQUIRE(!emp::Has(v1, 4));
  REQUIRE(emp::Product(v1) == 180);
  REQUIRE(emp::Slice(v1,1,3) == emp::vector<int>({2,3}));

  // Test handling vector-of-vectors.
  using vv_int_t = emp::vector< emp::vector< int > >;
  vv_int_t vv = {{1,2,3},{4,5,6},{7,8,9}};
  vv_int_t vv2 = emp::Transpose(vv);
  REQUIRE(vv[0][2] == 3);
  REQUIRE(vv[1][0] == 4);
  REQUIRE(vv2[0][2] == 7);
  REQUIRE(vv2[1][0] == 2);
}

// DEFINE_ATTR(Foo);
// DEFINE_ATTR(Bar);
// DEFINE_ATTR(Bazz);

struct ident_t {
  template <typename T>
  constexpr decltype(auto) operator()(T&& value) const {
    return std::forward<T>(value);
  }
};

constexpr ident_t ident{};

template <typename T>
struct Callable {
  T value;

  constexpr decltype(auto) operator()() & { return value; }
  constexpr decltype(auto) operator()() const & { return value; }
  constexpr decltype(auto) operator()() && { return std::move(value); }
  constexpr decltype(auto) operator()() const && { return std::move(value); }
};
template <typename T>
constexpr Callable<std::decay_t<T>> callable(T&& value) {
  return {std::forward<T>(value)};
}

struct NoCopy {
  int value;
  constexpr NoCopy(int value) : value(value) {}
  constexpr NoCopy(const NoCopy&) = delete;
  constexpr NoCopy(NoCopy&&) = default;

  constexpr NoCopy& operator=(const NoCopy&) = delete;
  constexpr NoCopy& operator=(NoCopy&&) = default;
};
constexpr bool operator==(const NoCopy& a, const NoCopy& b) {
  return a.value == b.value;
}
std::ostream& operator<<(std::ostream& out, const NoCopy& nc) {
  return out << "NoCopy{" << nc.value << "}";
}

struct {
  template <typename I, typename T>
  constexpr auto operator()(I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value).Get();
  }

  template <typename I, typename T>
  constexpr auto operator()(const char* name, I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value);
  }
} sum;

struct {
  template <typename I, typename A, typename B>
  constexpr auto operator()(I&& init, A&& a, B&& b) const {
    return std::forward<I>(init) +
           (std::forward<A>(a).Get() * std::forward<B>(b).Get());
  }
} dot;

struct {
  template <typename I, typename T>
  constexpr NoCopy operator()(I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).Get().value};
  }

  template <typename I, typename T>
  constexpr NoCopy operator()(const char* name, I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).value};
  }
} sum_nocopy;

// TEST_CASE("Test Attribute Packs", "[tools]") {
//   using namespace emp::tools;
//   // Test Construction & access
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(6)).Get(), 6);
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(7))).Get(), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(7), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Foo(7), Bar(6)), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Bazz(7), Bar(6)), callable(0)), 0);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Foo(3), Foo(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2), Bazz(1)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3)), 3);

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(7), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Foo(7), Bar(6)), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Bazz(7), Bar(6)), 0), 0);

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(5), Bar(6)), Merge(Foo(5), Bar(6)));

//   // Test NoCopy
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(NoCopy{7})).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(NoCopy{7}))).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(NoCopy{7}), callable(NoCopy{0})),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{7})),
//     NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{0})),
//     NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Bar(NoCopy{2}), Foo(NoCopy{3})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Foo(NoCopy{3}), Foo(NoCopy{2})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2}), Bazz(NoCopy{1})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(NoCopy{2}), Foo(NoCopy{3})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3})), NoCopy{3});

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(NoCopy{7}), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(NoCopy{5}), Bar(NoCopy{6})),
//                        Merge(Foo(NoCopy{5}), Bar(NoCopy{6})));

//   // Test Mapping
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(6), Foo(ident))(NoCopy{5}),
//                        Merge(Foo(NoCopy{5}), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(NoCopy{6}), Foo(ident))(5),
//                        Merge(Foo(5), Bar(NoCopy{6})));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(5), Foo(6)).Reduce(0, sum), 11);
//   CONSTEXPR_REQUIRE_EQ(
//     Merge(Bar(NoCopy{5}), Foo(NoCopy{6})).Reduce(NoCopy{0}, sum_nocopy),
//     NoCopy{11});
//   CONSTEXPR_REQUIRE_EQ(MergeReduce(0, sum, Bar(6), Foo(7)), 6 + 7);
//   // CONSTEXPR_REQUIRE_EQ(MergeReduce(0, [](auto init, auto& a, auto& b) {return
//   // init + a.Get() * b.Get();}, Bar(6) + Foo(7), Bar(11) + Foo(12)),
//   //                      6 + 7);
// }
