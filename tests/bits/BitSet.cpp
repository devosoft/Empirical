
#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include <sstream>
#include <fstream>
#include <string>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>
#include <ratio>

#include "emp/data/DataNode.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/functional/FunctionSet.hpp"
#include "emp/compiler/RegEx.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/TypeTracker.hpp"
#include "emp/tools/attrs.hpp"
#include "emp/base/map.hpp"


/// Ensures that
/// 1) A == B
/// 2) A and B can be constexprs or non-contexprs.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_REQUIRE_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    REQUIRE(A == B);                     \
  }



/**
 * Status booleans (Any, All, None)
 * as well as Clear and SetAll
 */
void test_status(){
  emp::BitSet<10> bs10;
  REQUIRE(!bs10.any());
  REQUIRE(bs10.none());
  REQUIRE(!bs10.all());
  bs10.SetAll();
  REQUIRE(bs10.all());
  bs10.Clear();
  REQUIRE(bs10.none());
}

/**
 * GetSize
 */
void test_size(){
  emp::BitSet<42> bs42;
  REQUIRE(bs42.size() == 42);

  emp::BitSet<35> bs35;
  REQUIRE(bs35.GetSize() == 35);

  emp::BitSet<1> bs1;
  REQUIRE(bs1.size() == 1);
}

/**
 * Flip and Toggle
 */
void test_flip(){
  emp::BitSet<2> bs2;	// bs2 = 00
  bs2.flip(0);				// bs2 = 01
  REQUIRE(bs2[0]);

  emp::BitSet<8> bs8;	// bs8 = 00000000
  bs8.flip(0,4);			// bs8 = 00001111
  REQUIRE(bs8[0]);
  REQUIRE(bs8[1]);
  REQUIRE(bs8[2]);
  REQUIRE(bs8[3]);
  REQUIRE(!bs8[4]);

  bs8[0].Toggle();		// bs8 = 00001110
  REQUIRE(!bs8[0]);

  emp::BitSet<4> bs4;	// bs4 = 0000
  bs4.flip();					// bs4 = 1111
  REQUIRE(bs4.all());
}

/**
 * FindBit and PopBit
 */
void test_find(){
  emp::BitSet<10> bs10;	// bs10 = 00 00000000
  bs10.flip(3);					// bs10 = 00 00001000
  REQUIRE(bs10.FindBit() == 3);
  bs10.PopBit();				// bs10 = 00 00000000
  REQUIRE(bs10.PopBit() == -1);
  bs10.flip(3);
  bs10.flip(1);
  REQUIRE(bs10.FindBit(2) == 3);
  REQUIRE(bs10.FindBit(4) == -1);
}

/**
 * GetByte and SetByte
 */
void test_byte(){
  emp::BitSet<10>	bs10;
  bs10.SetByte(0, 10);
  REQUIRE(bs10.GetByte(0) == 10);

  bs10.flip(0,4);
  REQUIRE(bs10.GetByte(0) == 5);
  bs10.SetByte(1, 3);
  REQUIRE(bs10.count() == 4);
}

/**
 * GetBytes
 */
// actual testing function
template <size_t Bits>
void do_byte_test() {
  emp::BitSet<Bits> bs;

  for (size_t i = 0; i < Bits / 8; ++i) {
    bs.SetByte(i, 10 * i);
  }

  const auto myspan = bs.GetBytes();
  for (size_t i = 0; i < Bits / 8; ++i) {
    REQUIRE(myspan[i] == static_cast<std::byte>(i * 10));
  }
}
// helper function that uses a fold expression to
// unpack the integer sequence of bits to test
// and then call the actual testing function with each as a template arg
template <typename T, T... Bits>
void do_byte_tests(const std::integer_sequence<T, Bits...>& sequence) {
  ((do_byte_test<Bits>()),...);
}
// function that holds what number of bits to test, and then calls
// the helper function with them
void test_bytes() {
  // sequence of number of bits to test
  std::index_sequence<16, 17, 32, 33, 64, 65, 128, 129> bits_to_test{};
  do_byte_tests(bits_to_test);
}
/**
 * Left and Right shifts
 */
void test_shift(){
  emp::BitSet<40> bs40;
  bs40.SetByte(0, 1);
  bs40 <<= 34;
  REQUIRE(bs40.GetByte(4) == 4);

  emp::BitSet<10> bs10;
  bs10.SetByte(0, 10);
  bs10 >>= 2;
  REQUIRE(bs10.GetByte(0) == 2);
}

/**
 * Count ones
 */
void test_count(){
  emp::BitSet<12> bs12;
  bs12.SetAll();
  REQUIRE(bs12.count() == 12);
  REQUIRE(bs12.CountOnes_Sparse() == 12);
  bs12.flip(0,5);
  REQUIRE(bs12.count() == 7);
}

/**
 * Get ones
 */
void test_get_ones(){
  emp::BitSet<5> bs5;
  bs5.flip(2); // 00100
  emp::vector<size_t> ones = bs5.GetOnes();
  REQUIRE(ones.size() == 1);
  REQUIRE(ones[0] == 2);
}


/**
 * Get and Set bits
 */
void test_bit(){
  emp::BitSet<8> bs8;
  bs8.Set(0, 1); 			// bs8 = 00000001
  REQUIRE(bs8.Get(0));

  bs8.Set(7, 1);			// bs8 = 10000001
  bs8.Set(0, 0);			// bs8 = 10000000
  REQUIRE(!bs8.Get(0));
  REQUIRE(bs8.Get(7));
}

/**
 * Bitwise XOR ^
 */
void test_bitwise_xor(){
  emp::BitSet<4> bs4;
  bs4.Set(0, 1);
  emp::BitSet<4> bs4_1;
  bs4_1.SetByte(0,3);
  bs4 ^= bs4_1; 								// bs4 = 0001 ^ 0011 = 0010
  REQUIRE(bs4.GetByte(0) == 2); 	// 0010 = 2
  bs4_1.PopBit(); 							// bs4_1 = 0010
  bs4 ^= bs4_1; 								// bs4 = 0010 ^ 0010 = 0000
  REQUIRE(bs4.GetByte(0) == 0);	// 0000 = 0
}

/**
 * Bitwise OR |
 */
void test_bitwise_or(){
  emp::BitSet<10> bs10;
  emp::BitSet<10> bs10_1;
  bs10.Set(1,1); 								// bs10 = 00 0000 0010
  bs10_1.Set(3,1);
  bs10_1.SetByte(1,3);
  REQUIRE(bs10_1.count() == 3);	// bs10_1 = 11 00001000
  bs10_1 |= bs10;								// bs10_1 = 11 00001000 | 00 00000010 = 11 00001010
  REQUIRE(bs10_1.GetByte(0) == 10);
  REQUIRE(bs10_1.GetByte(1) == 3);
}

/**
 * Bitwise AND &
 */
void test_bitwise_and(){
  emp::BitSet<8> bs8;
  emp::BitSet<8> bs8_1;
  bs8.SetByte(0,13);		// bs8 = 00001101
  bs8_1.SetByte(0,10);	// bs8_1 = 00001010
  bs8_1 &= bs8;					// bs8_1 = 00001010 & 00001101 = 00001000
  REQUIRE(bs8_1.GetByte(0) == 8);
}

/**
 * NAND, NOR, EQU & SELF
 */
void test_more_comparators(){
  // NAND
  emp::BitSet<8> bs8_1;
  emp::BitSet<8> bs8_2;
  bs8_1.SetAll();
  REQUIRE(bs8_1.NAND(bs8_2).All());
  bs8_2.flip(1);
  bs8_1.NAND_SELF(bs8_2);
  REQUIRE(bs8_1.Any());
  REQUIRE( !(bs8_1.Get(1)) );

  // NOR
  bs8_1.SetAll();
  bs8_2.Clear();
  REQUIRE(bs8_1.NOR(bs8_2).None());
  bs8_1.flip(1);
  bs8_1.NOR_SELF(bs8_2);
  REQUIRE(bs8_1.Get(1));

  // EQU
  bs8_1.Clear();
  bs8_2.SetAll();
  REQUIRE( (bs8_1.EQU(bs8_2).None()) );
  bs8_2.Clear();
  bs8_2.EQU_SELF(bs8_1);
  REQUIRE(bs8_2.All());
}

/**
 * Random bitset
 */
void test_random(){
  emp::Random rndm;
  emp::BitSet<8> bs8(rndm);
  bs8.Randomize(rndm, 1);
  REQUIRE(bs8.all());
  bs8.Randomize(rndm, 0);
  REQUIRE(bs8.none());
}

/**
 * Copy
 */
void test_copy(){
  emp::BitSet<10> bs10;
  bs10.SetAll();
  bs10.flip(0,5);

  emp::BitSet<10> bs10_1;
  bs10_1 = bs10;
  REQUIRE(bs10 == bs10_1);
}

/**
 * Comparators (>=,>,==,!=,<,<=)
 */
void test_comparators(){
  emp::BitSet<10> bs10;
  emp::BitSet<10> bs10_1;
  bs10_1.SetAll();
  REQUIRE(bs10_1 != bs10);
  REQUIRE(bs10_1 > bs10);
  bs10.SetAll();
  REQUIRE(bs10_1 >= bs10);
  REQUIRE(bs10_1 <= bs10);
  REQUIRE(bs10_1 == bs10);
  REQUIRE(!(bs10_1 < bs10));
  bs10.Clear();
  REQUIRE( (bs10 < bs10_1) );
}

/**
 * Export
 */
void test_export(){
  emp::BitSet<8> bs8;
  bs8.SetAll();
  REQUIRE(bs8.count() == 8);
  emp::BitSet<10> bs10 = bs8.Export<10>();
  REQUIRE(bs10.size() == 10);
  REQUIRE(bs10.GetByte(0) == 255);
  REQUIRE(bs10.GetByte(1) == 0);
}

/**
 * Import
 */
void test_import(){
  emp::BitSet<8> bs8;
  emp::BitSet<20> bs20;
  bs20[5] = 1;

  bs8.Import(bs20);
  REQUIRE(bs8[5]);

  emp::BitSet<10> bs10;
  bs10.SetAll();
  bs20.Import(bs10);
  REQUIRE(bs20.count() == 10);
}

/**
 * Print
 */
void test_print(){
  emp::BitSet<8> bs8;
  bs8.SetAll();
  bs8.Set(1, false);

  std::stringstream ss;
  bs8.Print(ss);
  REQUIRE(ss.str() == "11111101");
  ss.str(std::string());

  ss << bs8;
  REQUIRE(ss.str() == "11111101");
  ss.str(std::string());

  bs8.PrintArray(ss);
  REQUIRE(ss.str() == "10111111");
  ss.str(std::string());

  bs8.Clear();
  bs8.Set(1, true);
  bs8.Set(4, true);
  bs8.PrintOneIDs(ss);
  REQUIRE(ss.str() == "1 4 ");
}


TEST_CASE("Test BitSet", "[bits]")
{
  test_status();
  test_size();
  test_flip();
  test_bit();
  test_byte();
  test_bytes();
  test_find();
  test_count();
  test_get_ones();
  test_copy();
  test_shift();
  test_comparators();
  test_bitwise_or();
  test_bitwise_xor();
  test_bitwise_and();
  test_more_comparators();
  test_export();
  test_import();
  test_print();
}


// For BitSet Import/Export
template <size_t SOURCE_BITS, size_t DEST_BITS>
struct ImportExportTester {

  static void test() {

    emp::Random rand(1);

    // using default parameter
    emp::BitSet<SOURCE_BITS> source(rand);
    emp::BitSet<DEST_BITS> dest(rand);

    dest.template Import(source);

    for(size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for(size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    dest.Clear();
    dest = source.template Export<dest.GetSize()>();

    for(size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for(size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    // using all from_bit's
    source.Randomize(rand);
    dest.Randomize(rand);

    for(size_t from_bit = 0; from_bit < source.GetSize(); ++from_bit) {
      // std::cout << "---------" << std::endl;
      // std::cout << source << std::endl;
      dest.template Import(source, from_bit);
      // std::cout << "=========" << std::endl;
      // std::cout << from_bit << std::endl;
      // std::cout << source << std::endl;
      // std::cout << dest << std::endl;
      for(size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for(size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

      dest.Clear();
      dest = source.template Export<dest.GetSize()>(from_bit);

      for(size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for(size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

    }
  }
};

// for BitSet ROTATE_SELF
// adapted from spraetor.github.io/2015/12/26/compile-time-loops.html
// TODO: replace with https://en.cppreference.com/w/cpp/utility/integer_sequence
template <size_t N>
struct MultiTester2 {

  template <size_t I>
  static void test() {

    emp::Random rand(1);

    constexpr int W = N - 2;
    emp::BitSet<W> bs;

    for (int j = 0; j < W; ++j) {
      bs.Clear(); bs.Set(j);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c1 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c2 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c2);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Clear(); bs.Set(j);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c3 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c3);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c4 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c4);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));
    }

    if constexpr (I+1 < N) {
      // recurse
      MultiTester2<N>::test<I+1>();
    }
  }

};

// TODO: replace with https://en.cppreference.com/w/cpp/utility/integer_sequence
template <int N>
struct MultiTester {

  template <int I>
  static void test() {

    constexpr int width = I;

    emp::Random rand(1);
    emp::BitSet<width> bs(rand);
    const emp::BitSet<width> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -width - 1; i <= width + 1; ++i) {
      for (size_t rep = 0; rep < width; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }

    for (int i = -width - 1; i <= width + 1; ++i) {
      // for large widths, just do one starting position
      for (int j = 0; j < (width < 200 ? width : 1); ++j) {
        bs.Clear(); bs.Set(j);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == 1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.SetAll(); bs.Set(j, false);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == width-1);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j);
        const size_t c1 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j, false);
        const size_t c2 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c2);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));
      }
    }

    if constexpr (N < 200) {
      // test templated rotates (only for small N)
      MultiTester2<I+2>::template test<0>();
    }

    if constexpr (I+1 < N && N < 200) {
      // recurse
      MultiTester<N>::test<I+1>();
    } else if constexpr (I+1 < N ) {
      // recurse
      MultiTester<N>::test<I+63>();
    }
  }

};

template class emp::BitSet<5>;
TEST_CASE("Another Test BitSet", "[bits]")
{

  // test BitSet GetSize, GetNumBytes
  {
    REQUIRE(emp::BitSet<2>{}.GetSize() == 2);
    REQUIRE(emp::BitSet<2>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<7>{}.GetSize() == 7);
    REQUIRE(emp::BitSet<7>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<8>{}.GetSize() == 8);
    REQUIRE(emp::BitSet<8>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<9>{}.GetSize() == 9);
    REQUIRE(emp::BitSet<9>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitSet<16>{}.GetSize() == 16);
    REQUIRE(emp::BitSet<16>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitSet<24>{}.GetSize() == 24);
    REQUIRE(emp::BitSet<24>{}.GetNumBytes() == 3);
  }

  // test BitSet reverse
  {

    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF() == emp::BitSet<1>{0});
    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF().CountOnes() == 0);
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF() == emp::BitSet<1>{1});
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF().CountOnes() == 1);

    REQUIRE(
      (emp::BitSet<2>{1,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,1})
    );
    REQUIRE((emp::BitSet<2>{1,1}.REVERSE_SELF().CountOnes()) == 2);
    REQUIRE(
      (emp::BitSet<2>{0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,0})
    );
    REQUIRE((emp::BitSet<2>{0,1}.REVERSE_SELF().CountOnes()) == 1);
    REQUIRE(
      (emp::BitSet<2>{0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{0,0})
    );
    REQUIRE((emp::BitSet<2>{0,0}.REVERSE_SELF().CountOnes()) == 0);

    REQUIRE(
      (emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<8>{1,0,1,0,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<8>{0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,0,0,0,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      3
    );
    REQUIRE(
      (emp::BitSet<9>{1,0,1,0,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1})
    );
    REQUIRE(
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes())
      ==
      4
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,1,1,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      6
    );

    emp::Random rand(1);
    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<15> bs(rand);
      bs[0] = 0;
      bs[15-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<16> bs(rand);
      bs[0] = 0;
      bs[16-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<17> bs(rand);
      bs[0] = 0;
      bs[17-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<31> bs(rand);
      bs[0] = 0;
      bs[31-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<32> bs(rand);
      bs[0] = 0;
      bs[32-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<33> bs(rand);
      bs[0] = 0;
      bs[33-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<63> bs(rand);
      bs[0] = 0;
      bs[63-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<64> bs(rand);
      bs[0] = 0;
      bs[64-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<65> bs(rand);
      bs[0] = 0;
      bs[65-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<127> bs(rand);
      bs[0] = 0;
      bs[127-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<128> bs(rand);
      bs[0] = 0;
      bs[128-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<129> bs(rand);
      bs[0] = 0;
      bs[129-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

  }

  // test BitSet addition
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, std::numeric_limits<uint32_t>::max() - 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1);
  bs0+=bs1;
  REQUIRE (bs0.GetUInt(0) == 4294967295);
  REQUIRE ((bs0+bs1).GetUInt(0) == 0);
  REQUIRE ((bs0+bs0).GetUInt(0) == 4294967294);

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, emp::IntPow(2UL, 8UL)-1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);
  REQUIRE((bs2+bs3).GetUInt(0) == 0);
  emp::BitSet<64> bs4;
  bs4.SetUInt(0, std::numeric_limits<uint32_t>::max()-1);
  bs4.SetUInt(1, std::numeric_limits<uint32_t>::max());
  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);
  }

  // test BitSet subtraction
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0, 1);
  bs0 = bs0 - bs1;
  REQUIRE (bs0.GetUInt(0) == 0);
  REQUIRE ((bs0-bs1).GetUInt(0) == std::numeric_limits<uint32_t>::max());

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, 1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);

  bs2-=bs3;
  REQUIRE (bs2.GetUInt(0) == 0);
  REQUIRE((bs2-bs3).GetUInt(0) == emp::IntPow(2UL,8UL)-1);

  emp::BitSet<64> bs4;
  bs4.SetUInt(0, 1);
  bs4.SetUInt(1, 0);

  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  bs4 = bs4 - bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  }

  // test addition and subtraction with multiple fields
  {
  emp::BitSet<65> bs1;
  emp::BitSet<65> bs2;

  /* PART 1 */
  bs1.Clear();
  bs2.Clear();

  bs1.Set(64); // 10000...
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 64; ++i) REQUIRE((bs1 - bs2).Get(i));
  REQUIRE(!(bs1 - bs2).Get(64));

  bs1 -= bs2;

  for(size_t i = 0; i < 64; ++i) {
    REQUIRE(bs1.Get(i));
  }
  REQUIRE(!bs1.Get(64));

  /* PART 2 */
  bs1.Clear();
  bs2.Clear();

  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 65; ++i) REQUIRE((bs1 - bs2).Get(i));

  bs1 -= bs2;

  for(size_t i = 0; i < 65; ++i) REQUIRE(bs1.Get(i));

  /* PART 3 */
  bs1.Clear();
  bs2.Clear();

  for(size_t i = 0; i < 65; ++i) bs1.Set(i); // 11111...11111
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 65; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  for(size_t i = 0; i < 65; ++i) REQUIRE(!(bs2 + bs1).Get(i));

  bs1 += bs2;

  for(size_t i = 0; i < 65; ++i) REQUIRE(!bs1.Get(i));

  /* PART 4 */
  bs1.Clear();
  bs2.Clear();

  for(size_t i = 0; i < 64; ++i) bs1.Set(i); // 01111...11111
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 64; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  REQUIRE((bs1 + bs2).Get(64));
  for(size_t i = 0; i < 64; ++i) REQUIRE(!(bs2 + bs1).Get(i));
  REQUIRE((bs2 + bs1).Get(64));

  bs1 += bs2;

  for(size_t i = 0; i < 64; ++i) REQUIRE(!bs1.Get(i));
  REQUIRE((bs2 + bs1).Get(64));
  }

  // test GetDouble() and MaxDouble()
  {
  emp::BitSet<3> bs0{0,0,0};
  REQUIRE(bs0.GetDouble() == 0.0);
  REQUIRE(bs0.MaxDouble() == 7.0);

  emp::BitSet<3> bs1{0,0,1};
  REQUIRE(bs1.GetDouble() == 1.0);

  emp::BitSet<3> bs2{0,1,1};
  REQUIRE(bs2.GetDouble() == 3.0);

  emp::BitSet<3> bs3{1,1,1};
  REQUIRE(bs3.GetDouble() == 7.0);

  emp::BitSet<3> bs4{1,1,0};
  REQUIRE(bs4.GetDouble() == 6.0);

  emp::BitSet<32> bs5;
  bs5.SetUInt(0, 1789156UL);
  REQUIRE(bs5.GetDouble() == 1789156ULL);
  REQUIRE(bs5.MaxDouble() == 4294967295.0);

  emp::BitSet<64> bs6;
  bs6.SetUInt64(0, 1789156816848ULL);
  REQUIRE(bs6.GetDouble() == 1789156816848ULL);
  REQUIRE(bs6.MaxDouble() == 18446744073709551615.0);

  emp::BitSet<65> bs7;
  bs7.SetUInt64(0, 1789156816848ULL);
  bs7.Set(64);
  REQUIRE(bs7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));
  REQUIRE(bs7.MaxDouble() == 36893488147419103231.0);

  emp::BitSet<1027> bs8;
  bs8.Set(1026);
  REQUIRE(std::isinf(bs8.GetDouble()));
  REQUIRE(std::isinf(bs8.MaxDouble()));
  }

  // test list initializer
  {
  emp::BitSet<3> bs_empty{0,0,0};
  emp::BitSet<3> bs_first{1,0,0};
  emp::BitSet<3> bs_last{0,0,1};
  emp::BitSet<3> bs_full{1,1,1};

  REQUIRE(bs_empty.CountOnes() == 0);
  REQUIRE(bs_first.CountOnes() == 1);
  REQUIRE(bs_last.CountOnes() == 1);
  REQUIRE(bs_full.CountOnes() == 3);
  }

  // test Import and Export
  {

    emp::Random rand(1);

    emp::BitSet<32> orig(rand);

    emp::array<emp::BitSet<32>, 1> d1;
    emp::array<emp::BitSet<16>, 2> d2;
    emp::array<emp::BitSet<8>, 4> d4;
    emp::array<emp::BitSet<4>, 8> d8;
    emp::array<emp::BitSet<2>, 16> d16;
    emp::array<emp::BitSet<1>, 32> d32;

    // Import

    d1[0].Import(orig, 0);
    for(size_t i = 0; i < 2; ++i) d2[i].Import(orig, i * 16);
    for(size_t i = 0; i < 4; ++i) d4[i].Import(orig, i * 8);
    for(size_t i = 0; i < 8; ++i) d8[i].Import(orig, i * 4);
    for(size_t i = 0; i < 16; ++i) d16[i].Import(orig, i * 2);
    for(size_t i = 0; i < 32; ++i) d32[i].Import(orig, i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // Export

    d1[0] = orig.Export<32>(0);
    for(size_t i = 0; i < 2; ++i) d2[i] = orig.Export<16>(i * 16);
    for(size_t i = 0; i < 4; ++i) d4[i] = orig.Export<8>(i * 8);
    for(size_t i = 0; i < 8; ++i) d8[i] = orig.Export<4>(i * 4);
    for(size_t i = 0; i < 16; ++i) d16[i] = orig.Export<2>(i * 2);
    for(size_t i = 0; i < 32; ++i) d32[i] = orig.Export<1>(i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // now test some funky imports and exports
    // interesting container sizes:
    // 1, 17, 29, 32, 33, 64, 65, 95, 128, 129

    ImportExportTester<1,1>::test();
    ImportExportTester<1,17>::test();
    ImportExportTester<1,29>::test();
    ImportExportTester<1,32>::test();
    ImportExportTester<1,33>::test();
    ImportExportTester<1,64>::test();
    ImportExportTester<1,65>::test();
    ImportExportTester<1,96>::test();
    ImportExportTester<1,128>::test();
    ImportExportTester<1,129>::test();

    ImportExportTester<17,1>::test();
    ImportExportTester<17,17>::test();
    ImportExportTester<17,29>::test();
    ImportExportTester<17,32>::test();
    ImportExportTester<17,33>::test();
    ImportExportTester<17,64>::test();
    ImportExportTester<17,65>::test();
    ImportExportTester<17,96>::test();
    ImportExportTester<17,128>::test();
    ImportExportTester<17,129>::test();

    ImportExportTester<29,1>::test();
    ImportExportTester<29,17>::test();
    ImportExportTester<29,29>::test();
    ImportExportTester<29,32>::test();
    ImportExportTester<29,33>::test();
    ImportExportTester<29,64>::test();
    ImportExportTester<29,65>::test();
    ImportExportTester<29,96>::test();
    ImportExportTester<29,128>::test();
    ImportExportTester<29,129>::test();

    ImportExportTester<32,1>::test();
    ImportExportTester<32,17>::test();
    ImportExportTester<32,29>::test();
    ImportExportTester<32,32>::test();
    ImportExportTester<32,33>::test();
    ImportExportTester<32,64>::test();
    ImportExportTester<32,65>::test();
    ImportExportTester<32,96>::test();
    ImportExportTester<32,128>::test();
    ImportExportTester<32,129>::test();

    ImportExportTester<33,1>::test();
    ImportExportTester<33,17>::test();
    ImportExportTester<33,29>::test();
    ImportExportTester<33,32>::test();
    ImportExportTester<33,33>::test();
    ImportExportTester<33,64>::test();
    ImportExportTester<33,65>::test();
    ImportExportTester<33,96>::test();
    ImportExportTester<33,128>::test();
    ImportExportTester<33,129>::test();

    ImportExportTester<64,1>::test();
    ImportExportTester<64,17>::test();
    ImportExportTester<64,29>::test();
    ImportExportTester<64,32>::test();
    ImportExportTester<64,33>::test();
    ImportExportTester<64,64>::test();
    ImportExportTester<64,65>::test();
    ImportExportTester<64,96>::test();
    ImportExportTester<64,128>::test();
    ImportExportTester<64,129>::test();

    ImportExportTester<65,1>::test();
    ImportExportTester<65,17>::test();
    ImportExportTester<65,29>::test();
    ImportExportTester<65,32>::test();
    ImportExportTester<65,33>::test();
    ImportExportTester<65,64>::test();
    ImportExportTester<65,65>::test();
    ImportExportTester<65,96>::test();
    ImportExportTester<65,128>::test();
    ImportExportTester<65,129>::test();

    ImportExportTester<96,1>::test();
    ImportExportTester<96,17>::test();
    ImportExportTester<96,29>::test();
    ImportExportTester<96,32>::test();
    ImportExportTester<96,33>::test();
    ImportExportTester<96,64>::test();
    ImportExportTester<96,65>::test();
    ImportExportTester<96,96>::test();
    ImportExportTester<96,128>::test();
    ImportExportTester<96,129>::test();

    ImportExportTester<128,1>::test();
    ImportExportTester<128,17>::test();
    ImportExportTester<128,29>::test();
    ImportExportTester<128,32>::test();
    ImportExportTester<128,33>::test();
    ImportExportTester<128,64>::test();
    ImportExportTester<128,65>::test();
    ImportExportTester<128,96>::test();
    ImportExportTester<128,128>::test();
    ImportExportTester<128,129>::test();

    ImportExportTester<129,1>::test();
    ImportExportTester<129,17>::test();
    ImportExportTester<129,29>::test();
    ImportExportTester<129,32>::test();
    ImportExportTester<129,33>::test();
    ImportExportTester<129,64>::test();
    ImportExportTester<129,65>::test();
    ImportExportTester<129,96>::test();
    ImportExportTester<129,128>::test();
    ImportExportTester<129,129>::test();

  }

  emp::BitSet<10> bs10;
  emp::BitSet<25> bs25;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  bs80 <<= 1;
  emp::BitSet<80> bs80c(bs80);

  for (size_t i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  bs80.Clear();

  REQUIRE(bs10[2] == false);
  bs10.flip(2);
  REQUIRE(bs10[2] == true);

  REQUIRE(bs32[2] == false);
  bs32.flip(2);
  REQUIRE(bs32[2] == true);

  REQUIRE(bs80[2] == false);
  bs80.flip(2);
  REQUIRE(bs80[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == false);}
  bs10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == true);}
  REQUIRE(bs10[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == false);}
  bs32.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == true);}
  REQUIRE(bs32[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == false);}
  bs80.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == true);}
  REQUIRE(bs80[8] == false);

  bs80[70] = 1;

  REQUIRE(bs10.GetUInt(0) == 252);
  REQUIRE(bs10.GetUInt32(0) == 252);
  REQUIRE(bs10.GetUInt64(0) == 252);

  REQUIRE(bs32.GetUInt(0) == 252);
  REQUIRE(bs32.GetUInt32(0) == 252);
  REQUIRE(bs32.GetUInt64(0) == 252);

  REQUIRE(bs80.GetUInt(0) == 252);
  REQUIRE(bs80.GetUInt(1) == 0);
  REQUIRE(bs80.GetUInt(2) == 64);
  REQUIRE(bs80.GetUInt32(0) == 252);
  REQUIRE(bs80.GetUInt32(1) == 0);
  REQUIRE(bs80.GetUInt32(2) == 64);
  REQUIRE(bs80.GetUInt64(0) == 252);
  REQUIRE(bs80.GetUInt64(1) == 64);

  bs80 = bs80c;

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUIntAtBit(64) == 130);
  REQUIRE(bs80.GetValueAtBit<5>(64) == 2);

  emp::BitSet<96> bs;

  REQUIRE (bs.LongestSegmentOnes() == 0);
  bs.SetUInt(2, 1);
  REQUIRE (bs.LongestSegmentOnes() == 1);
  bs.SetUInt(1, 3);
  REQUIRE (bs.LongestSegmentOnes() == 2);
  bs.SetUInt(0, 7);
  REQUIRE (bs.LongestSegmentOnes() == 3);

  bs.SetUInt(0, std::numeric_limits<uint32_t>::max());
  bs.SetUInt(1, std::numeric_limits<uint32_t>::max() - 1);
  bs.SetUInt(2, std::numeric_limits<uint32_t>::max() - 3);
  REQUIRE (bs.LongestSegmentOnes() == 32);

  // tests for ROTATE
  // ... with one set bit
  bs10.Clear(); bs10.Set(0);
  bs25.Clear(); bs25.Set(0);
  bs32.Clear(); bs32.Set(0);
  bs50.Clear(); bs50.Set(0);
  bs64.Clear(); bs64.Set(0);
  bs80.Clear(); bs80.Set(0);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs10.CountOnes() == bs10.ROTATE(rot).CountOnes() );
    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 10) REQUIRE( bs10 != bs10.ROTATE(rot) );
    else REQUIRE( bs10 == bs10.ROTATE(rot) );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

  }

  // ... with random set bits
  emp::Random rand(1);
  // no bs10 because there's a reasonable chance
  // of breaking the test's assumption of nonsymmetry
  bs25.Randomize(rand);
  bs32.Randomize(rand);
  bs50.Randomize(rand);
  bs64.Randomize(rand);
  bs80.Randomize(rand);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

  }

  // tests for ROTATE_SELF, ROTR_SELF, ROTL_SELF
  MultiTester<2>::test<1>();
  MultiTester<18>::test<17>();
  MultiTester<34>::test<31>();
  MultiTester<51>::test<50>();
  MultiTester<66>::test<63>();
  MultiTester<96>::test<93>();
  MultiTester<161>::test<160>();
  MultiTester<2050>::test<2048>();

  // tests for Mutate
  {
    emp::Random rando(1);
    emp::BitSet<25> bs_25;
    emp::BitSet<32> bs_32;
    emp::BitSet<50> bs_50;
    emp::BitSet<64> bs_64;
    emp::BitSet<80> bs_80;

    bs_25.Mutate(rando, 0);
    REQUIRE(!bs_25.CountOnes());

    bs_32.Mutate(rando, 0);
    REQUIRE(!bs_32.CountOnes());

    bs_50.Mutate(rando, 0);
    REQUIRE(!bs_50.CountOnes());

    bs_64.Mutate(rando, 0);
    REQUIRE(!bs_64.CountOnes());

    bs_80.Mutate(rando, 0);
    REQUIRE(!bs_80.CountOnes());


    bs_25.Mutate(rando, 1);
    REQUIRE( bs_25.CountOnes() == 1);

    bs_32.Mutate(rando, 1);
    REQUIRE( bs_32.CountOnes() == 1);

    bs_50.Mutate(rando, 1);
    REQUIRE( bs_50.CountOnes() == 1);

    bs_64.Mutate(rando, 1);
    REQUIRE( bs_64.CountOnes() == 1);

    bs_80.Mutate(rando, 1);
    REQUIRE( bs_80.CountOnes() == 1);

    bs_25.Clear();
    bs_32.Clear();
    bs_50.Clear();
    bs_64.Clear();
    bs_80.Clear();

    for (size_t i = 1; i < 5000; ++i) {
      bs_25.Mutate(rando, 1);
      REQUIRE(bs_25.CountOnes() <= i);

      bs_32.Mutate(rando, 1);
      REQUIRE(bs_32.CountOnes() <= i);

      bs_50.Mutate(rando, 1);
      REQUIRE(bs_50.CountOnes() <= i);

      bs_64.Mutate(rando, 1);
      REQUIRE(bs_64.CountOnes() <= i);

      bs_80.Mutate(rando, 1);
      REQUIRE(bs_80.CountOnes() <= i);
    }

    REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
    REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);
    REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
    REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);
    REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
    REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);
    REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
    REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);
    REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
    REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);

    for (size_t i = 0; i < 10; ++i) {
      bs_25.Mutate(rando, bs_25.size());
      REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
      REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);

      bs_32.Mutate(rando, bs_32.size());
      REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
      REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);

      bs_50.Mutate(rando, bs_50.size());
      REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
      REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);

      bs_64.Mutate(rando, bs_64.size());
      REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
      REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);

      bs_80.Mutate(rando, bs_80.size());
      REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
      REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);
    }
  }

  // serialize / deserialize
  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::BinaryOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::BinaryInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::JSONOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::JSONInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

}

TEST_CASE("Test BitSet timing", "[bits]")
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

TEST_CASE("Test BitSet string construction", "[tools]") {

  REQUIRE( emp::BitSet<5>( "01001" ) == emp::BitSet<5>{0, 1, 0, 0, 1} );
  REQUIRE(
    emp::BitSet<5>( std::bitset<5>( "01001" ) )
    == emp::BitSet<5>{0, 1, 0, 0, 1}
  );

}
