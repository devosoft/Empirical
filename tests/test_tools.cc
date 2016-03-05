#define EMP_TRACK_MEM
#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#undef NDEBUG
#define TDEBUG 1

#include <array>
#include <sstream>
#include <string>
#include <vector>


#include "../tools/BitMatrix.h"
#include "../tools/BitSet.h"
#include "../tools/BitVector.h"
#include "../tools/DynamicStringSet.h"
#include "../tools/functions.h"
#include "../tools/FunctionSet.h"
#include "../tools/Graph.h"
#include "../tools/graph_utils.h"
//#include "../tools/grid.h"
#include "../tools/mem_track.h"
#include "../tools/Ptr.h"
#include "../tools/Random.h"
#include "../tools/reflection.h"
#include "../tools/sequence_utils.h"
#include "../tools/serialize.h"
#include "../tools/string_utils.h"
#include "../tools/Trait.h"
#include "../tools/tuple_struct.h"
#include "../tools/vector.h"
#include "../tools/macro_math.h"
#include "../tools/macros.h"
#include "../tools/assert.h"
#include "../tools/unit_tests.h"
// currently these have no coveage; we include them so we get metrics on them
// this doesn't actually work--TODO: figure out why this doesn't work
#include "../tools/alert.h"
#include "../tools/const.h"
#include "../tools/errors.h"
#include "../tools/class.h"
#include "../tools/fixed.h"
#include "../tools/SolveState.h"
#include "../tools/ProbSchedule.h"
#include "../tools/serialize_macros.h"


// "Macros testing macros...Oh dear..."
#undef EMP_TEST_MACRO
#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                             \
  do {                                                                  \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));           \
    REQUIRE(result == EXP_RESULT);                                      \
  } while (false)


TEST_CASE("Test bitvectors", "[tools]")
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
  REQUIRE(bm45.FindBit() == bm45.GetID(1,2));

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

TEST_CASE("test BitSet", "[tools]")
{
  emp::BitSet<10> bs10;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  emp::BitSet<80> bs80c(bs80);
  bs80 <<= 1;

  for (int i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  
  // Test importing....
  bs10.Import(bs80 >> 70);

  REQUIRE(bs10.GetUInt(0) == 2);

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUIntAtBit(64) == 130);
  REQUIRE(bs80.GetValueAtBit<5>(64) == 2);
}


TEST_CASE("test BitSet timing", "[tools]")
{
  const int set_size = 100000;
  typedef emp::BitSet<set_size> TEST_TYPE;

  TEST_TYPE set1;
  TEST_TYPE set2;

  for (int i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  int total = 0;
  
  // should probably assert that this does what we want it to do...
  for (int i = 0; i < 100000; i++) {
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


TEST_CASE( "Testing BitVectors", "[tools]")
{
  emp::BitVector bv10(10);
  emp::BitVector bv32(32);
  emp::BitVector bv50(50);
  emp::BitVector bv64(64);
  emp::BitVector bv80(80);

  bv80[70] = 1;
  emp::BitVector bv80c(bv80);

  bv80 <<= 1;

  for (int i = 0; i < 75; i += 2) {
    emp::BitVector shift_vector = bv80 >> i;
    REQUIRE((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  bv10 = (bv80 >> 70);

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  REQUIRE(bv80.GetUIntAtBit(64) == 130);
  REQUIRE(bv80.GetValueAtBit<5>(64) == 2);  
}

TEST_CASE("Test BitVector timing", "[tools]")
{

  const int set_size = 100000;
  typedef emp::BitVector TEST_TYPE;

  TEST_TYPE set1(set_size);
  TEST_TYPE set2(set_size);

  for (int i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  int total = 0;
  for (int i = 0; i < 100000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  
  double time = 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC;
  //REQUIRE(time < 9000); // NOTE: WILL VARY INTENSELY ON VARIOUS SYSTEMS
  // SHOULD PROBABLY CHANGE

}

TEST_CASE("Test DynamicStringSet", "[tools]")
{
  emp::DynamicStringSet test_set;

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


char result_char;
void TestFun(int x, int y, char z) {
  result_char = z + x*y;
}

TEST_CASE("Test functions", "[tools]")
{

  bool test_bool = true;
  emp::toggle(test_bool);
  REQUIRE(test_bool == false);

  REQUIRE(emp::mod(10, 7) == 3);
  REQUIRE(emp::mod(3, 7) == 3);
  REQUIRE(emp::mod(-4, 7) == 3);
  REQUIRE(emp::mod(-11, 7) == 3);

  REQUIRE(emp::pow(2,3) == 8);
  REQUIRE(emp::pow(-2,2) == 4);
  REQUIRE(emp::pow(3,4) == 81);

  REQUIRE(emp::to_range(-10000, 10, 20) == 10);
  REQUIRE(emp::to_range(9, 10, 20) == 10);
  REQUIRE(emp::to_range(10, 10, 20) == 10);
  REQUIRE(emp::to_range(11, 10, 20) == 11);
  REQUIRE(emp::to_range(17, 10, 20) == 17);
  REQUIRE(emp::to_range(20, 10, 20) == 20);
  REQUIRE(emp::to_range(21, 10, 20) == 20);
  REQUIRE(emp::to_range(12345678, 10, 20) == 20);
  REQUIRE(emp::to_range<double>(12345678, 10, 20.1) == 20.1);
  REQUIRE(emp::to_range(12345678.0, 10.7, 20.1) == 20.1);

  
  // TEST FOR VARIADIC HELPER FUNCTIONS:

  REQUIRE((emp::get_type_index<char, char, bool, int, double>()) == 0);
  REQUIRE((emp::get_type_index<int, char, bool, int, double>()) == 2);
  REQUIRE((emp::get_type_index<double, char, bool, int, double>()) == 3);
  REQUIRE((emp::get_type_index<std::string, char, bool, int, double>()) < 0);

  REQUIRE((emp::has_unique_first_type<int, bool, std::string, bool, char>()) == true);
  REQUIRE((emp::has_unique_first_type<bool, int, std::string, bool, char>()) == false);
  REQUIRE((emp::has_unique_types<bool, int, std::string, std::vector<bool>, char>()) == true);
  REQUIRE((emp::has_unique_types<int, bool, std::string, bool, char>()) == false);


  std::tuple<int, int, char> test_tuple(3,2,'a');
  emp::ApplyTuple(TestFun, test_tuple);
  
  REQUIRE(result_char == 'g');

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
  emp::FunctionSet<void, int,int> fun_set;
  fun_set.Add(&fun1);
  fun_set.Add(&fun2);
  fun_set.Add(&fun3);
  fun_set.Add(&fun4);

  fun_set.Run(10, 2);

  REQUIRE(global_var1 == 12);
  REQUIRE(global_var2 == 8);
  REQUIRE(global_var3 == 20);
  REQUIRE(global_var4 == 5);

  // Test 2: Functions with non-void returns.
  emp::FunctionSet<double, double> fun_set2;
  fun_set2.Add(&fun5);
  fun_set2.Add(&fun6);
  fun_set2.Add(&fun7);

  std::vector<double> out_vals;

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

// TODO: add asserts
TEST_CASE("Test Graph utils", "[tools]")
{
  emp::Random random;
  // emp::Graph graph( emp::build_graph_tree(20, random) );
  // emp::Graph graph( emp::build_graph_random(20, 40, random) );
  emp::Graph graph( emp::build_graph_grid(5, 4, random) );

  // graph.PrintSym();
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
  std::vector<TestClass1 *> test_v;
  TestClass2 class2_mem;

  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 0);

  for (int i = 0; i < 1000; i++) {
    test_v.push_back( new TestClass1 );
  }

  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 1000);


  for (int i = 500; i < 1000; i++) {
    delete test_v[i];
  }

  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 500);
  //REQUIRE(EMP_TRACK_STATUS == 0);
 
}

TEST_CASE("Test Ptr", "[tools]")
{ 
  emp::PtrTracker<char>::Get();
  emp::PtrTracker<int>::Get();

  // Test default constructor.
  emp::Ptr<int> ptr1;
  ptr1.New();
  *ptr1 = 5;
  REQUIRE(*ptr1 == 5);
  ptr1.Delete();
  
  // Test pointer constructor
  int * temp_int = new int;
  emp::Ptr<int> ptr2(temp_int);
  *ptr2 = 10;
  REQUIRE(*ptr2 == 10);
  ptr2.Delete();

  // Test non-pointer object constructor
  int base_val = 15;
  emp::Ptr<int> ptr3(base_val);
  REQUIRE(*ptr3 == 15);
  base_val = 20;                 // Make sure pointed to value changes with original variable.
  REQUIRE(*ptr3 == 20);

  // Test copy-constructor.
  emp::Ptr<int> ptr4(ptr3);
  REQUIRE(*ptr4 == 20);
  *ptr4 = 25;                    // Change this new pointer...
  REQUIRE(*ptr4 == 25);       // ...make sure it actually changed.
  REQUIRE(*ptr3 == 25);       // ...make sure the other pointer reflects the change.
  REQUIRE(base_val == 25);    // ...make sure the original variable changed.

  // -- Test count tracking on emp::Ptr --
  // A bit of an odd set of test... we need to create and destory pointers to make sure
  // that all of the counts are correct, so we're going to use arrays of pointers to them.

  std::vector<emp::Ptr<char> *> ptr_set(10);
  ptr_set[0] = new emp::Ptr<char>;
  ptr_set[0]->New(42);
  for (int i = 1; i < 10; i++) ptr_set[i] = new emp::Ptr<char>(*(ptr_set[0]));

  // Do we have a proper count of 10?
  REQUIRE(ptr_set[0]->DebugGetCount() == 10);
  ptr_set[1]->New(91);
  REQUIRE(ptr_set[0]->DebugGetCount() == 9);
  *(ptr_set[2]) = *(ptr_set[1]);
  REQUIRE(ptr_set[0]->DebugGetCount() == 8);
  REQUIRE(ptr_set[1]->DebugGetCount() == 2);

  ptr_set[3]->Delete();

  // std::cout << ptr_set[0]->DebugGetCount() << std::endl;

  // @CAO Make sure we don't delete below 0
  // @CAO Make sure we don't delete below 1 if we own it
  // @CAO Make sure we only delete if you own it
  // @CAO Make sure not to delete twice!
  // @CAO Make sure we don't add (as owner) a pointer we already own

  // -- Do some direct tests on pointer trackers --

  int * real_ptr1 = new int(1);  // Count of 2 in tracker
  int * real_ptr2 = new int(2);  // Deleted in tracker
  int * real_ptr3 = new int(3);  // Unknown to tracker
  int * real_ptr4 = new int(4);  // Passively known to tracker (marked non-owner)
  emp::PtrTracker<int> & tracker = emp::PtrTracker<int>::Get();
  
  tracker.New(real_ptr1);
  tracker.Inc(real_ptr1);
  tracker.Inc(real_ptr1);
  tracker.Dec(real_ptr1);

  tracker.New(real_ptr2);
  tracker.MarkDeleted(real_ptr2);

  tracker.Old(real_ptr4);

  REQUIRE(tracker.HasPtr(real_ptr1) == true);
  REQUIRE(tracker.HasPtr(real_ptr2) == true);
  REQUIRE(tracker.HasPtr(real_ptr3) == false);
  REQUIRE(tracker.HasPtr(real_ptr4) == true);

  REQUIRE(tracker.IsActive(real_ptr1) == true);
  REQUIRE(tracker.IsActive(real_ptr2) == false);
  REQUIRE(tracker.IsActive(real_ptr3) == false);
  REQUIRE(tracker.IsActive(real_ptr4) == true);

  REQUIRE(tracker.IsOwner(real_ptr1) == true);
  REQUIRE(tracker.IsOwner(real_ptr2) == true);
  REQUIRE(tracker.IsOwner(real_ptr3) == false);
  REQUIRE(tracker.IsOwner(real_ptr4) == false);

  REQUIRE(tracker.GetCount(real_ptr1) == 2);
  REQUIRE(tracker.GetCount(real_ptr2) == 1);
  REQUIRE(tracker.GetCount(real_ptr3) == 0);
  REQUIRE(tracker.GetCount(real_ptr4) == 1);
}



TEST_CASE("Test random", "[tools]")
{
  emp::Random rng;

  // Test GetDouble with the law of large numbers.
  std::vector<int> val_counts(10);
  for (int i = 0; i < (int) val_counts.size(); i++) val_counts[i] = 0;

  const int num_tests = 100000;
  const double min_value = 2.5;
  const double max_value = 8.7;
  double total = 0.0;
  for (int i = 0; i < num_tests; i++) {
    const double cur_value = rng.GetDouble(min_value, max_value);
    total += cur_value;
    val_counts[(int) cur_value]++;
  }

  {
    const double expected_mean = (min_value + max_value) / 2.0;
    const double min_threshold = (expected_mean*0.997);
    const double max_threshold = (expected_mean*1.004);
    double mean_value = total/(double) num_tests;

    REQUIRE(mean_value > min_threshold);
    REQUIRE(mean_value < max_threshold);
  }

  // Test GetInt
  for (int i = 0; i < (int) val_counts.size(); i++) val_counts[i] = 0;
  total = 0.0;

  for (int i = 0; i < num_tests; i++) {
    const int cur_value = rng.GetInt((int) min_value, (int) max_value);
    total += cur_value;
    val_counts[cur_value]++;
  }

  {
    const double expected_mean = (double) (((int) min_value) + ((int) max_value) - 1) / 2.0;
    const double min_threshold = (expected_mean*0.997);
    const double max_threshold = (expected_mean*1.004);
    double mean_value = total/(double) num_tests;
    
    REQUIRE(mean_value > min_threshold);
    REQUIRE(mean_value < max_threshold);
  }
  
  // Test P
  double flip_prob = 0.56789;
  int hit_count = 0;
  for (int i = 0; i < num_tests; i++) {
    if (rng.P(flip_prob)) hit_count++;
  }

  double actual_prob = ((double) hit_count) / (double) num_tests;

  REQUIRE(actual_prob < flip_prob + 0.005);
  REQUIRE(actual_prob > flip_prob - 0.005);

  
  // Mimimal test of Choose()
  std::vector<int> choices = rng.Choose(100,10);

  REQUIRE(choices.size() == 10);
}

struct TestTrue {
  int test_member;
  int TestFun(int a, int b) { return a*b; }
};

struct TestFalse {
  int other_stuff;
};

struct TestTrueMethod {
  int test_member() { return 4; }
};

template <typename T>
int TestExternalFun(T & obj, int a, int b) { return a+b; }

EMP_CREATE_MEMBER_DETECTOR(test_member);
EMP_CREATE_METHOD_FALLBACK(DynamicFun, TestFun, TestExternalFun, int);

TEST_CASE("Test reflection", "[tools]")
{

  REQUIRE(EMP_Detect_test_member<TestTrue>::value == 1);
  REQUIRE(EMP_Detect_test_member<TestFalse>::value == 0);
  REQUIRE(EMP_Detect_test_member<TestTrueMethod>::value == 1);

  TestTrue t;
  TestFalse f;
  REQUIRE(DynamicFun(t, 20, 20) == 400);
  REQUIRE(DynamicFun(f, 20, 20) == 40);

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

  std::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  std::vector<int> v2 = { 1,4,5,6,8 };

  emp::align(v1,v2,0);

  REQUIRE((v2 == std::vector<int>({1,0,0,4,5,6,0,8,0})));
}



struct SerializeTest {
  int a;
  float b;        // unimportant data!
  std::string c;
  
  SerializeTest(int _a, float _b, std::string _c) : a(_a), b(_b), c(_c) { ; }
  EMP_SETUP_DATAPOD(SerializeTest, a, c);
};

struct SerializeTest_D : public SerializeTest {
  char d = '$';

  SerializeTest_D(int _a, float _b, std::string _c, char _d)
    : SerializeTest(_a, _b, _c), d(_d) { ; }
  EMP_SETUP_DATAPOD_D(SerializeTest_D, SerializeTest, d);
};

struct ExtraBase {
  double e;

  ExtraBase(double _e) : e(_e) { ; }
  EMP_SETUP_DATAPOD(ExtraBase, e);
};

struct MultiTest : public SerializeTest, public ExtraBase {
  bool f;

  MultiTest(int _a, float _b, std::string _c, double _e, bool _f)
    : SerializeTest(_a, _b, _c), ExtraBase(_e), f(_f) { ; }
  EMP_SETUP_DATAPOD_D2(MultiTest, SerializeTest, ExtraBase, f);
};

struct NestedTest {
  SerializeTest st;
  std::string name;
  SerializeTest_D std;
  MultiTest mt;

  NestedTest(int a1, float b1, std::string c1,
             int a2, float b2, std::string c2, char d2,
             int a3, float b3, std::string c3, double e3, bool f3)
    : st(a1, b1, c1), name("my_class"), std(a2, b2, c2, d2), mt(a3, b3, c3, e3, f3) { ; }

  EMP_SETUP_DATAPOD(NestedTest, st, name, std, mt);
};

struct BuiltInTypesTest {
  const int a;
  std::vector<int> int_v;

  BuiltInTypesTest(int _a, int v_size) : a(_a), int_v(v_size) {
    for (int i = 0; i < v_size; i++) int_v[i] = i*i;
  }

  EMP_SETUP_DATAPOD(BuiltInTypesTest, a, int_v);
};

TEST_CASE("Test serialize", "[tools]")
{
  std::stringstream ss;
  emp::serialize::DataPod pod(ss);


  // Basic test...

  SerializeTest st(7, 2.34, "my_test_string");
  st.EMP_Store(pod);

  SerializeTest st2(pod);

  REQUIRE(st2.a == 7);                 // Make sure a was reloaded correctly.
  REQUIRE(st2.c == "my_test_string");  // Make sure c was reloaded correctly.


  // Derived class Test

  SerializeTest_D stD(10,0.2,"three",'D');
  stD.EMP_Store(pod);

  SerializeTest_D stD2(pod);

  REQUIRE(stD2.a == 10);
  REQUIRE(stD2.c == "three");
  REQUIRE(stD2.d == 'D');

  // Multiply-derived class Test

  MultiTest stM(111,2.22,"ttt",4.5,true);
  stM.EMP_Store(pod);

  MultiTest stM2(pod);


  REQUIRE(stM2.a == 111);
  REQUIRE(stM2.c == "ttt");
  REQUIRE(stM2.e == 4.5);
  REQUIRE(stM2.f == true);


  // Nested objects test...

  NestedTest nt(91, 3.14, "magic numbers",
                100, 0.01, "powers of 10", '1',
                1001, 1.001, "ones and zeros", 0.125, true);
  nt.EMP_Store(pod);

  NestedTest nt2(pod);

  REQUIRE(nt2.st.a == 91);
  REQUIRE(nt2.st.c == "magic numbers");
  REQUIRE(nt2.name == "my_class");
  REQUIRE(nt2.std.a == 100);
  REQUIRE(nt2.std.c == "powers of 10");
  REQUIRE(nt2.std.d == '1');
  REQUIRE(nt2.mt.a == 1001);
  REQUIRE(nt2.mt.c == "ones and zeros");
  REQUIRE(nt2.mt.e == 0.125);
  REQUIRE(nt2.mt.f == true);


  // If we made it this far, everything must have worked!;

  const int v_size = 43;
  BuiltInTypesTest bitt(91, v_size);
  bitt.EMP_Store(pod);


  BuiltInTypesTest bitt2(pod);
}



TEST_CASE("Test string utils", "[tools]")
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

  std::vector<std::string> slices;
  emp::slice_string(base_string, slices, 's');

  REQUIRE(slices.size() == 5);
  REQUIRE(slices[1] == " i");
  REQUIRE(slices[3] == "t ");


  std::string cat_a = "ABC";
  bool cat_b = true;
  char cat_c = '2';
  int cat_d = 3;

  std::string cat_full = emp::to_string(cat_a, cat_b, cat_c, cat_d);

  REQUIRE(cat_full == "ABC123");
}



TEST_CASE("Test trait", "[tools]")
{
  emp::TraitManager<int, double, std::vector<bool>, char, std::string> tm;

  tm.AddTrait<int>("test_trait", "This is a test trait", 42);
  tm.AddTrait<std::string>("test2", "This is technically our second test trait.", "VALUE");
  tm.AddTrait<int>("test3", "And we need another int trait to test", 1000);


  //emp::TraitSet trait_set(tm);
}


TEST_CASE("Test vector", "[tools]")
{
  emp::vector<int> v(20);

  for (int i = 0; i < 20; i++) {
    v[i] = i * i;
  }

  int total = 0;
  for (int i : v) {
    total += i;
  }

  REQUIRE(total == 2470);
}


TEST_CASE("Test macro math", "[tools]")
{

  // Test converting between binary, decimal, and sum formats.
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(9), "0, 0, 0, 0, 0, 0, 1, 0, 0, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(91), "0, 0, 0, 1, 0, 1, 1, 0, 1, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(999), "1, 1, 1, 1, 1, 0, 0, 1, 1, 1");

  EMP_TEST_MACRO( EMP_BIN_TO_DEC(0,0,0,0,0,0,1,0,1,1), "11");
  EMP_TEST_MACRO( EMP_BIN_TO_DEC(0,0,0,1,0,1,1,0,1,1), "91");
  EMP_TEST_MACRO( EMP_BIN_TO_DEC(1,0,1,0,1,0,1,0,1,0), "682");

  EMP_TEST_MACRO( EMP_BIN_TO_SUM(0,0,0,1,0,1,1,0,1,1), "0, 0, 0, 64, 0, 16, 8, 0, 2, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_SUM(91), "0, 0, 0, 64, 0, 16, 8, 0, 2, 1");

  EMP_TEST_MACRO( EMP_BIN_TO_PACK(0,0,0,1,0,1,1,0,1,1), "(64, 16, 8, 2, 1)");
  EMP_TEST_MACRO( EMP_DEC_TO_PACK(91), "(64, 16, 8, 2, 1)");
  
  // Test Boolean logic
  EMP_TEST_MACRO( EMP_NOT(0), "1" );
  EMP_TEST_MACRO( EMP_NOT(EMP_NOT(0)), "0" );
  
  EMP_TEST_MACRO( EMP_BIT_EQU(0,0), "1" );
  EMP_TEST_MACRO( EMP_BIT_EQU(0,1), "0" );
  EMP_TEST_MACRO( EMP_BIT_EQU(1,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_EQU(1,1), "1" );

  EMP_TEST_MACRO( EMP_BIT_LESS(0,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_LESS(0,1), "1" );
  EMP_TEST_MACRO( EMP_BIT_LESS(1,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_LESS(1,1), "0" );

  EMP_TEST_MACRO( EMP_BIT_GTR(0,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_GTR(0,1), "0" );
  EMP_TEST_MACRO( EMP_BIT_GTR(1,0), "1" );
  EMP_TEST_MACRO( EMP_BIT_GTR(1,1), "0" );

  // Test conditionals.
  EMP_TEST_MACRO( EMP_IF_impl_0(abc), "~, abc" );
  EMP_TEST_MACRO( EMP_IF_impl_1(abc), "EMP_IF_impl_1(abc)" );
  EMP_TEST_MACRO( EMP_IF(0, A, B), "B" );
  EMP_TEST_MACRO( EMP_IF(1, A, B), "A" );
  
  // Test comparisons
  EMP_TEST_MACRO( EMP_COMPARE(10,20), "B" );
  EMP_TEST_MACRO( EMP_COMPARE(1023,1022), "A" );
  EMP_TEST_MACRO( EMP_COMPARE(1000,999), "A" );
  EMP_TEST_MACRO( EMP_COMPARE(678,678), "X" );

  EMP_TEST_MACRO( EMP_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_EQU(2,5), "0" );
  EMP_TEST_MACRO( EMP_EQU(5,8), "0" );
  EMP_TEST_MACRO( EMP_EQU(8,5), "0" );
  EMP_TEST_MACRO( EMP_EQU(5,2), "0" );

  EMP_TEST_MACRO( EMP_LESS(5,5), "0" );
  EMP_TEST_MACRO( EMP_LESS(2,5), "1" );
  EMP_TEST_MACRO( EMP_LESS(5,8), "1" );
  EMP_TEST_MACRO( EMP_LESS(8,5), "0" );
  EMP_TEST_MACRO( EMP_LESS(5,2), "0" );

  EMP_TEST_MACRO( EMP_LESS_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(2,5), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(5,8), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(8,5), "0" );
  EMP_TEST_MACRO( EMP_LESS_EQU(5,2), "0" );

  EMP_TEST_MACRO( EMP_GTR(5,5), "0" );
  EMP_TEST_MACRO( EMP_GTR(2,5), "0" );
  EMP_TEST_MACRO( EMP_GTR(5,8), "0" );
  EMP_TEST_MACRO( EMP_GTR(8,5), "1" );
  EMP_TEST_MACRO( EMP_GTR(5,2), "1" );

  EMP_TEST_MACRO( EMP_GTR_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_GTR_EQU(2,5), "0" );
  EMP_TEST_MACRO( EMP_GTR_EQU(5,8), "0" );
  EMP_TEST_MACRO( EMP_GTR_EQU(8,5), "1" );
  EMP_TEST_MACRO( EMP_GTR_EQU(5,2), "1" );

  EMP_TEST_MACRO( EMP_NEQU(5,5), "0" );
  EMP_TEST_MACRO( EMP_NEQU(2,5), "1" );
  EMP_TEST_MACRO( EMP_NEQU(5,8), "1" );
  EMP_TEST_MACRO( EMP_NEQU(8,5), "1" );
  EMP_TEST_MACRO( EMP_NEQU(5,2), "1" );


  
  // Test other helper math functions.
  EMP_TEST_MACRO( EMP_MATH_VAL_TIMES_0(222), "0" );
  EMP_TEST_MACRO( EMP_MATH_VAL_TIMES_1(222), "222" );

  EMP_TEST_MACRO( EMP_MATH_BIN_TIMES_0(0,0,1,0,1,0,1,0,1,0), "0, 0, 0, 0, 0, 0, 0, 0, 0, 0" );
  EMP_TEST_MACRO( EMP_MATH_BIN_TIMES_1(0,0,1,0,1,0,1,0,1,0), "0, 0, 1, 0, 1, 0, 1, 0, 1, 0" );

  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(1, 1), "2");
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(1, N), "0");
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(0, N), "N");

  EMP_TEST_MACRO( EMP_MATH_GET_CARRY(2), "1");
  EMP_TEST_MACRO( EMP_MATH_CLEAR_CARRY(2), "0");
  
  // Now in combination...
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(1), EMP_MATH_GET_CARRY(2)), "2" );

  // Basic Addition...
  EMP_TEST_MACRO( EMP_ADD(1, 2), "3");
  EMP_TEST_MACRO( EMP_ADD(5, 5), "10");
  EMP_TEST_MACRO( EMP_ADD(7, 7), "14");
  EMP_TEST_MACRO( EMP_ADD(111, 112), "223");
  EMP_TEST_MACRO( EMP_ADD(127, 1), "128");
  EMP_TEST_MACRO( EMP_ADD(123, 789), "912");
  EMP_TEST_MACRO( EMP_ADD(1023, 1), "0");      // Overflow

  EMP_TEST_MACRO( EMP_ADD_10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10), "55" );
  
  // Basic Subtraction...
  EMP_TEST_MACRO( EMP_SUB(10, 7), "3");
  EMP_TEST_MACRO( EMP_SUB(128, 1), "127");
  EMP_TEST_MACRO( EMP_SUB(250, 250), "0");
  EMP_TEST_MACRO( EMP_SUB(250, 100), "150");
  EMP_TEST_MACRO( EMP_SUB(91, 66), "25");
  EMP_TEST_MACRO( EMP_SUB(99, 100), "1023");   // Underflow

  // Combination of add and sub
  EMP_TEST_MACRO( EMP_ADD( EMP_SUB(250, 100), EMP_SUB(91, 66)), "175");

  // Shifting
  EMP_TEST_MACRO( EMP_SHIFTL(17), "34");
  EMP_TEST_MACRO( EMP_SHIFTL(111), "222");  
  EMP_TEST_MACRO( EMP_SHIFTL(444), "888");  
  EMP_TEST_MACRO( EMP_SHIFTL(1023), "1022");   // Overflow...

  EMP_TEST_MACRO( EMP_SHIFTR(100), "50");
  EMP_TEST_MACRO( EMP_SHIFTR(151), "75");

  EMP_TEST_MACRO( EMP_SHIFTL_X(0, 700), "700");
  EMP_TEST_MACRO( EMP_SHIFTL_X(5, 17),  "544");
  EMP_TEST_MACRO( EMP_SHIFTL_X(1, 111), "222");  
  EMP_TEST_MACRO( EMP_SHIFTR_X(1, 100), "50");
  EMP_TEST_MACRO( EMP_SHIFTR_X(3, 151), "18");

  // Inc, dec, half...
  EMP_TEST_MACRO( EMP_INC(20), "21");
  EMP_TEST_MACRO( EMP_INC(55), "56");
  EMP_TEST_MACRO( EMP_INC(63), "64");
  EMP_TEST_MACRO( EMP_INC(801), "802");

  EMP_TEST_MACRO( EMP_DEC(20), "19");
  EMP_TEST_MACRO( EMP_DEC(55), "54");
  EMP_TEST_MACRO( EMP_DEC(63), "62");
  EMP_TEST_MACRO( EMP_DEC(900), "899");

  EMP_TEST_MACRO( EMP_HALF(17), "8");
  EMP_TEST_MACRO( EMP_HALF(18), "9");
  EMP_TEST_MACRO( EMP_HALF(60), "30");
  EMP_TEST_MACRO( EMP_HALF(1001), "500");

  // Multiply!
  EMP_TEST_MACRO( EMP_MULT(1, 1), "1");
  EMP_TEST_MACRO( EMP_MULT(200, 0), "0");
  EMP_TEST_MACRO( EMP_MULT(201, 1), "201");
  EMP_TEST_MACRO( EMP_MULT(10, 7), "70");
  EMP_TEST_MACRO( EMP_MULT(25, 9), "225");
  EMP_TEST_MACRO( EMP_MULT(65, 3), "195");
  EMP_TEST_MACRO( EMP_MULT(65, 15), "975");

  // Bit Manipulation!
  EMP_TEST_MACRO( EMP_COUNT_ONES(0), "0");
  EMP_TEST_MACRO( EMP_COUNT_ONES(509), "8");
  EMP_TEST_MACRO( EMP_COUNT_ONES(1023), "10");

  EMP_TEST_MACRO( EMP_LOG2(0), "0" );
  EMP_TEST_MACRO( EMP_LOG2(1), "1" );
  EMP_TEST_MACRO( EMP_LOG2(3), "2" );
  EMP_TEST_MACRO( EMP_LOG2(5), "3" );
  EMP_TEST_MACRO( EMP_LOG2(10), "4" );
  EMP_TEST_MACRO( EMP_LOG2(20), "5" );
  EMP_TEST_MACRO( EMP_LOG2(40), "6" );
  EMP_TEST_MACRO( EMP_LOG2(75), "7" );
  EMP_TEST_MACRO( EMP_LOG2(150), "8" );
  EMP_TEST_MACRO( EMP_LOG2(300), "9" );
  EMP_TEST_MACRO( EMP_LOG2(600), "10" );

  // Division!
  EMP_TEST_MACRO( EMP_DIV_start(2), "8" );
  EMP_TEST_MACRO( EMP_DIV(8, 2), "4" );
  EMP_TEST_MACRO( EMP_DIV(100, 5), "20" );
  EMP_TEST_MACRO( EMP_DIV(1000, 17), "58" );

  // Modulus!
  EMP_TEST_MACRO( EMP_MOD(10, 3), "1" );
  EMP_TEST_MACRO( EMP_MOD(127, 10), "7" );
  EMP_TEST_MACRO( EMP_MOD(127, 1000), "127" );
  EMP_TEST_MACRO( EMP_MOD(102, 3), "0" );
}



TEST_CASE("Test macros", "[tools]")
{
  EMP_TEST_MACRO( EMP_POP_ARGS_32(1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "3,4,5,6,7,8,9,0");
  EMP_TEST_MACRO( EMP_POP_ARGS(32, 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "3,4,5,6,7,8,9,0");
  EMP_TEST_MACRO( EMP_POP_ARGS(39, 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "0");

  // Test getting a specific argument.
  EMP_TEST_MACRO( EMP_POP_ARGS( EMP_DEC(5), 11,12,13,14,15,16,17 ), "15,16,17");
  EMP_TEST_MACRO( EMP_GET_ARG(5, 11,12,13,14,15,16,17), "15");

  // Test counting number of arguments.
  EMP_TEST_MACRO( EMP_COUNT_ARGS(a, b, c), "3" );
  EMP_TEST_MACRO( EMP_COUNT_ARGS(x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x), "600" );

  // Make sure EMP_STRINGIFY can process macros before running
  EMP_TEST_MACRO( EMP_STRINGIFY(EMP_MERGE(ab, c, de, f)), "\"abcdef\"");
  EMP_TEST_MACRO( EMP_STRINGIFY("abcdef"), "\"\\\"abcdef\\\"\"" );


  // Test PACK manipulation
  EMP_TEST_MACRO( EMP_PACK_ARGS(a,b,c), "(a,b,c)");
  EMP_TEST_MACRO( EMP_UNPACK_ARGS((a,b,c)), "a,b,c");
  EMP_TEST_MACRO( EMP_PACK_POP((a,b,c)), "(b,c)");
  EMP_TEST_MACRO( EMP_PACK_TOP((a,b,c)), "a");
  EMP_TEST_MACRO( EMP_PACK_PUSH(x, (a,b,c)), "(x,a,b,c)");
  EMP_TEST_MACRO( EMP_PACK_SIZE((a,b,c)), "3");

  // BAD TEST: EMP_TEST_MACRO( EMP_ARGS_TO_PACKS_1(4, a,b,c,d,e,f,g), "(a , b , c , d)" );

  
  EMP_TEST_MACRO( EMP_CALL_BY_PACKS(TST_, (Fixed), a,b,c,d,e,f,g,h,i,j,k,l,m), "TST_8((Fixed), a,b,c,d,e,f,g,h,i,j,k,l,m, ~) TST_4((Fixed), i,j,k,l,m, ~) TST_1((Fixed), m, ~)" );

  // Make sure we can wrap each argument in a macro.
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o] [p]" );
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o]" );
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m]" );


  // Test replacement of commas
  EMP_TEST_MACRO( EMP_REPLACE_COMMAS(~, x,x,x,x,x,x,x), "x ~ x ~ x ~ x ~ x ~ x ~ x" );
  EMP_TEST_MACRO( EMP_REPLACE_COMMAS(%, x,x,x,x,x,x,x,x), "x % x % x % x % x % x % x % x" );

  
  // Simple argument manipulation
  EMP_TEST_MACRO( EMP_ROTATE_ARGS(a, b, c), "b, c, a" );
  
  // Test trimming argument lists.
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b,c,d,e,f,g,h,i ), "a, c, d, f, g, i" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x), 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50 ), "1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49" );


  // Test more complex layouts...
  EMP_TEST_MACRO( EMP_LAYOUT(EMP_DECORATE, +, a, b, c, d, e, f, g, h), "[a] + [b] + [c] + [d] + [e] + [f] + [g] + [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARGS(EMP_DECORATE, a, b, c, d, e, f, g, h), "[a], [b], [c], [d], [e], [f], [g], [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARG_PAIRS(EMP_DECORATE_PAIR, A, a, B, b, C, c, D, d, E, e, F, f), "[A-a], [B-b], [C-c], [D-d], [E-e], [F-f]" );


  // Rest controlling argument number.
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(15, x), "x, x, x, x, x, x, x, x, x, x, x, x, x, x, x" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(5, x,y,z), "x,y,z, x,y,z, x,y,z, x,y,z, x,y,z" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(63, 123), "123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123" );

  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(26, x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x),
                  "x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x" );

  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(5, a, b, c, d, e, f, g, h, i, j), "a , b , c , d , e" );
  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(4, a, b, c, d, e, f, g, h, i, j), "a , b , c , d" );
  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(4, a, b, c, d), "a , b , c , d" );
  
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(3, x, a, b, c, d), "a , b , c" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(4, x, a, b, c, d), "a , b , c , d" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(7, x, a, b, c, d), "a , b , c , d , x , x , x" );

  // Test collect only-odd or only-even arguments.
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "1, 3, 5, 7, 9, 11");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "2, 4, 6, 8, 10, 12");
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "1, 3, 5, 7, 9, 11, 13");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "2, 4, 6, 8, 10, 12");


  EMP_TEST_MACRO( EMP_REVERSE_ARGS(a,b,c, d), "d, c, b, a" );
  EMP_TEST_MACRO( EMP_TYPES_TO_ARGS(int, char, bool, std::string),
                  "int arg1, char arg2, bool arg3, std::string arg4" );


  // Test EMP_STRINGIFY_EACH
  std::array<std::string, 2> test = {EMP_STRINGIFY_EACH(some, words)};
  std::array<std::string, 9> test9 = {EMP_STRINGIFY_EACH(one, two, three, four, five, six, seven, eight, nine)};

  REQUIRE(test.size() == 2);
  REQUIRE(test[0] == "some");
  REQUIRE(test[1] == "words");
  REQUIRE(test9.size() == 9);
  REQUIRE(test9[4] == "five");
  REQUIRE(test9[7] == "eight");

  EMP_TEST_MACRO( EMP_STRINGIFY_EACH(some, words), "\"some\", \"words\"" );
}

// no idea what these do, but they're probably necessary


TEST_CASE("Test assert", "[tools]")
{
  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(100);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(23 < 24);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert((14 < 13)?0:1);
  REQUIRE(emp::assert_last_fail == 0);


  // Now here are some that should FAIL
/*  emp_assert(false);
  EMP_TEST_VALUE(emp::assert_last_fail, "1");
  EMP_TEST_VALUE(emp::assert_fail_info.filename, "assert.cc");
  EMP_TEST_VALUE(emp::assert_fail_info.line_num, "31");
  EMP_TEST_VALUE(emp::assert_fail_info.error, "false");
*/
  // if (emp::assert_fail_info.filename != "assert.cc") std::cerr << "Failed case 6!" << std::endl;
  // if (emp::assert_fail_info.line_num != __LINE__ - 3) std::cerr << "Failed case 7!" << std::endl;
  // if (emp::assert_fail_info.error != "false") std::cerr << "Failed case 8!" << std::endl;
}
