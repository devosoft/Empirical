//===- llvm/unittest/ADT/SmallVectorTest.cpp ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// SmallVector unit tests.
//
//===----------------------------------------------------------------------===//

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "datastructs/SmallVector.h"

#include <list>
#include <stdarg.h>

// adapted from https://github.com/llvm-mirror/llvm/blob/1a7bc073dc2147d0df26e68e2074877afd5c3257/unittests/ADT/SmallVectorTest.cpp

// using namespace emp;
//
// namespace {
//
// /// A helper class that counts the total number of constructor and
// /// destructor calls.
// class Constructable {
// private:
//   static int numConstructorCalls;
//   static int numMoveConstructorCalls;
//   static int numCopyConstructorCalls;
//   static int numDestructorCalls;
//   static int numAssignmentCalls;
//   static int numMoveAssignmentCalls;
//   static int numCopyAssignmentCalls;
//
//   bool constructed;
//   int value;
//
// public:
//   Constructable() : constructed(true), value(0) {
//     ++numConstructorCalls;
//   }
//
//   Constructable(int val) : constructed(true), value(val) {
//     ++numConstructorCalls;
//   }
//
//   Constructable(const Constructable & src) : constructed(true) {
//     value = src.value;
//     ++numConstructorCalls;
//     ++numCopyConstructorCalls;
//   }
//
//   Constructable(Constructable && src) : constructed(true) {
//     value = src.value;
//     ++numConstructorCalls;
//     ++numMoveConstructorCalls;
//   }
//
//   ~Constructable() {
//     REQUIRE(constructed);
//     ++numDestructorCalls;
//     constructed = false;
//   }
//
//   Constructable & operator=(const Constructable & src) {
//     REQUIRE(constructed);
//     value = src.value;
//     ++numAssignmentCalls;
//     ++numCopyAssignmentCalls;
//     return *this;
//   }
//
//   Constructable & operator=(Constructable && src) {
//     REQUIRE(constructed);
//     value = src.value;
//     ++numAssignmentCalls;
//     ++numMoveAssignmentCalls;
//     return *this;
//   }
//
//   int getValue() const {
//     return abs(value);
//   }
//
//   static void reset() {
//     numConstructorCalls = 0;
//     numMoveConstructorCalls = 0;
//     numCopyConstructorCalls = 0;
//     numDestructorCalls = 0;
//     numAssignmentCalls = 0;
//     numMoveAssignmentCalls = 0;
//     numCopyAssignmentCalls = 0;
//   }
//
//   static int getNumConstructorCalls() {
//     return numConstructorCalls;
//   }
//
//   static int getNumMoveConstructorCalls() {
//     return numMoveConstructorCalls;
//   }
//
//   static int getNumCopyConstructorCalls() {
//     return numCopyConstructorCalls;
//   }
//
//   static int getNumDestructorCalls() {
//     return numDestructorCalls;
//   }
//
//   static int getNumAssignmentCalls() {
//     return numAssignmentCalls;
//   }
//
//   static int getNumMoveAssignmentCalls() {
//     return numMoveAssignmentCalls;
//   }
//
//   static int getNumCopyAssignmentCalls() {
//     return numCopyAssignmentCalls;
//   }
//
//   friend bool operator==(const Constructable & c0, const Constructable & c1) {
//     return c0.getValue() == c1.getValue();
//   }
//
//   friend bool operator!=(const Constructable & c0, const Constructable & c1) {
//     return c0.getValue() != c1.getValue();
//   }
// };
//
// int Constructable::numConstructorCalls;
// int Constructable::numCopyConstructorCalls;
// int Constructable::numMoveConstructorCalls;
// int Constructable::numDestructorCalls;
// int Constructable::numAssignmentCalls;
// int Constructable::numCopyAssignmentCalls;
// int Constructable::numMoveAssignmentCalls;
//
// struct NonCopyable {
//   NonCopyable() {}
//   NonCopyable(NonCopyable &&) {}
//   NonCopyable &operator=(NonCopyable &&) { return *this; }
// private:
//   NonCopyable(const NonCopyable &) = delete;
//   NonCopyable &operator=(const NonCopyable &) = delete;
// };
//
// void CompileTest() {
//   SmallVector<NonCopyable, 0> V;
//   V.resize(42);
// }
//
// // class SmallVectorTestBase : public testing::Test {
// // protected:
// //   void SetUp() override { Constructable::reset(); }
// //
// //   template <typename VectorT>
// //   void assertEmpty(VectorT & v) {
// //     // Size tests
// //     REQUIRE(0u == v.size());
// //     REQUIRE(v.empty());
// //
// //     // Iterator tests
// //     REQUIRE(v.begin() == v.end());
// //   }
// //
// //   // Assert that v contains the specified values, in order.
// //   template <typename VectorT>
// //   void assertValuesInOrder(VectorT & v, size_t size, ...) {
// //     REQUIRE(size == v.size());
// //
// //     va_list ap;
// //     va_start(ap, size);
// //     for (size_t i = 0; i < size; ++i) {
// //       int value = va_arg(ap, int);
// //       REQUIRE(value == v[i].getValue());
// //     }
// //
// //     va_end(ap);
// //   }
// //
// //   // Generate a sequence of values to initialize the vector.
// //   template <typename VectorT>
// //   void makeSequence(VectorT & v, int start, int end) {
// //     for (int i = start; i <= end; ++i) {
// //       v.push_back(Constructable(i));
// //     }
// //   }
// // };
// //
// // // Test fixture class
// // template <typename VectorT>
// // class SmallVectorTest : public SmallVectorTestBase {
// // protected:
// //   VectorT theVector;
// //   VectorT otherVector;
// // };
// //
// //
// // typedef ::testing::Types<SmallVector<Constructable, 0>,
// //                          SmallVector<Constructable, 1>,
// //                          SmallVector<Constructable, 2>,
// //                          SmallVector<Constructable, 4>,
// //                          SmallVector<Constructable, 5>
// //                          > SmallVectorTestTypes;
// // TYPED_TEST_CASE("SmallVectorTestTypes); TODO
//
// // Constructor test.
// TEST_CASE("ConstructorNonIter", "[tools]") {
//   this->theVector = SmallVector<Constructable, 2>(2, 2);
//   this->assertValuesInOrder(this->theVector, 2u, 2, 2);
// }
//
// // Constructor test.
// TEST_CASE("ConstructorIter", "[tools]") {
//   emp::SmallVector<int, 2> theVector;
//
//   int arr[] = {1, 2, 3};
//   theVector =
//       SmallVector<Constructable, 4>(std::begin(arr), std::end(arr));
//   this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
// }
//
// // New vector test.
// TEST_CASE("EmptyVector", "[tools]") {
//   // this->assertEmpty(this->theVector);
//   emp::SmallVector<int, 2> theVector;
//
//   REQUIRE(theVector.rbegin() == theVector.rend());
//   REQUIRE(0 == Constructable::getNumConstructorCalls());
//   REQUIRE(0 == Constructable::getNumDestructorCalls());
// }
//
// // Simple insertions and deletions.
// TEST_CASE("PushPop", "[tools]") {
//
//   emp::SmallVector<int, 2> theVector;
//
//   // Track whether the vector will potentially have to grow.
//   bool RequiresGrowth = theVector.capacity() < 3;
//
//   // Push an element
//   theVector.push_back(Constructable(1));
//
//   // Size tests
//   // this->assertValuesInOrder(theVector, 1u, 1);
//   REQUIRE_FALSE(theVector.begin() == theVector.end());
//   REQUIRE_FALSE(theVector.empty());
//
//   // Push another element
//   theVector.push_back(Constructable(2));
//   // this->assertValuesInOrder(theVector, 2u, 1, 2);
//
//   // Insert at beginning
//   theVector.insert(theVector.begin(), theVector[1]);
//   // this->assertValuesInOrder(theVector, 3u, 2, 1, 2);
//
//   // Pop one element
//   theVector.pop_back();
//   // this->assertValuesInOrder(theVector, 2u, 2, 1);
//
//   // Pop remaining elements
//   theVector.pop_back();
//   theVector.pop_back();
//   // this->assertEmpty(this->theVector);
//
//   // Check number of constructor calls. Should be 2 for each list element,
//   // one for the argument to push_back, one for the argument to insert,
//   // and one for the list element itself.
//   if (!RequiresGrowth) {
//     REQUIRE(5 == Constructable::getNumConstructorCalls());
//     REQUIRE(5 == Constructable::getNumDestructorCalls());
//   } else {
//     // If we had to grow the vector, these only have a lower bound, but should
//     // always be equal.
//     REQUIRE(5 <= Constructable::getNumConstructorCalls());
//     REQUIRE(
//       Constructable::getNumConstructorCalls(),
//       == Constructable::getNumDestructorCalls()
//     );
//   }
// }
//
// // Clear test.
// TEST_CASE("Clear", "[tools]") {
//
//   emp::SmallVector<int, 2> theVector;
//
//   theVector.reserve(2);
//   // makeSequence(theVector, 1, 2);
//   theVector.clear();
//
//   // assertEmpty(theVector);
//   // REQUIRE(4 == Constructable::getNumConstructorCalls());
//   // REQUIRE(4 == Constructable::getNumDestructorCalls());
// }
//
// // Resize smaller test.
// TEST_CASE("ResizeShrink", "[tools]") {
//
//   emp::SmallVector<int, 2> theVector;
//
//   theVector.reserve(3);
//   // this->makeSequence(theVector, 1, 3);
//   theVector.resize(1);
//
//   // this->assertValuesInOrder(theVector, 1u, 1);
//   // REQUIRE(6 == Constructable::getNumConstructorCalls());
//   // REQUIRE(5 == Constructable::getNumDestructorCalls());
// }
//
// // Resize bigger test.
// TEST_CASE("ResizeGrow", "[tools]") {
//
//   theVector.resize(2);
//
//   REQUIRE(2 == Constructable::getNumConstructorCalls());
//   REQUIRE(0 == Constructable::getNumDestructorCalls());
//   REQUIRE(2u == theVector.size());
// }
//
// TEST_CASE("ResizeWithElements", "[tools]") {
//   this->theVector.resize(2);
//
//   Constructable::reset();
//
//   this->theVector.resize(4);
//
//   size_t Ctors = Constructable::getNumConstructorCalls();
//   REQUIRE(Ctors == 2 || Ctors == 4);
//   size_t MoveCtors = Constructable::getNumMoveConstructorCalls();
//   REQUIRE(MoveCtors == 0 || MoveCtors == 2);
//   size_t Dtors = Constructable::getNumDestructorCalls();
//   REQUIRE(Dtors == 0 || Dtors == 2);
// }
//
// // Resize with fill value.
// TEST_CASE("ResizeFill", "[tools]") {
//
//   this->theVector.resize(3, Constructable(77));
//   this->assertValuesInOrder(this->theVector, 3u, 77, 77, 77);
// }
//
// // Overflow past fixed size.
// TEST_CASE("Overflow", "[tools]") {
//
//   // Push more elements than the fixed size.
//   this->makeSequence(this->theVector, 1, 10);
//
//   // Test size and values.
//   REQUIRE(10u == this->theVector.size());
//   for (int i = 0; i < 10; ++i) {
//     REQUIRE(i+1 == this->theVector[i].getValue());
//   }
//
//   // Now resize back to fixed size.
//   this->theVector.resize(1);
//
//   this->assertValuesInOrder(this->theVector, 1u, 1);
// }
//
// // Iteration tests.
// TEST_CASE("Iteration", "[tools]") {
//   this->makeSequence(this->theVector, 1, 2);
//
//   // Forward Iteration
//   typename TypeParam::iterator it = this->theVector.begin();
//   REQUIRE(*it == this->theVector.front());
//   REQUIRE(*it == this->theVector[0]);
//   REQUIRE(1 == it->getValue());
//   ++it;
//   REQUIRE(*it == this->theVector[1]);
//   REQUIRE(*it == this->theVector.back());
//   REQUIRE(2 == it->getValue());
//   ++it;
//   REQUIRE(it == this->theVector.end());
//   --it;
//   REQUIRE(*it == this->theVector[1]);
//   REQUIRE(2 == it->getValue());
//   --it;
//   REQUIRE(*it == this->theVector[0]);
//   REQUIRE(1 == it->getValue());
//
//   // Reverse Iteration
//   typename TypeParam::reverse_iterator rit = this->theVector.rbegin();
//   REQUIRE(*rit == this->theVector[1]);
//   REQUIRE(2 == rit->getValue());
//   ++rit;
//   REQUIRE(*rit == this->theVector[0]);
//   REQUIRE(1 == rit->getValue());
//   ++rit;
//   REQUIRE(rit == this->theVector.rend());
//   --rit;
//   REQUIRE(*rit == this->theVector[0]);
//   REQUIRE(1 == rit->getValue());
//   --rit;
//   REQUIRE(*rit == this->theVector[1]);
//   REQUIRE(2 == rit->getValue());
// }
//
// // Swap test.
// TEST_CASE("Swap", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 2);
//   std::swap(this->theVector, this->otherVector);
//
//   this->assertEmpty(this->theVector);
//   this->assertValuesInOrder(this->otherVector, 2u, 1, 2);
// }
//
// // Append test
// TEST_CASE("Append", "[tools]") {
//
//   this->makeSequence(this->otherVector, 2, 3);
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.append(this->otherVector.begin(), this->otherVector.end());
//
//   this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
// }
//
// // Append repeated test
// TEST_CASE("AppendRepeated", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.append(2, Constructable(77));
//   this->assertValuesInOrder(this->theVector, 3u, 1, 77, 77);
// }
//
// // Append test
// TEST_CASE("AppendNonIter", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.append(2, 7);
//   this->assertValuesInOrder(this->theVector, 3u, 1, 7, 7);
// }
//
// struct output_iterator {
//   typedef std::output_iterator_tag iterator_category;
//   typedef int value_type;
//   typedef int difference_type;
//   typedef value_type *pointer;
//   typedef value_type &reference;
//   operator int() { return 2; }
//   operator Constructable() { return 7; }
// };
//
// TEST_CASE("AppendRepeatedNonForwardIter", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.append(output_iterator(), output_iterator());
//   this->assertValuesInOrder(this->theVector, 3u, 1, 7, 7);
// }
//
// // Assign test
// TEST_CASE("Assign", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.assign(2, Constructable(77));
//   this->assertValuesInOrder(this->theVector, 2u, 77, 77);
// }
//
// // Assign test
// TEST_CASE("AssignRange", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   int arr[] = {1, 2, 3};
//   this->theVector.assign(std::begin(arr), std::end(arr));
//   this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
// }
//
// // Assign test
// TEST_CASE("AssignNonIter", "[tools]") {
//
//   this->theVector.push_back(Constructable(1));
//   this->theVector.assign(2, 7);
//   this->assertValuesInOrder(this->theVector, 2u, 7, 7);
// }
//
// // Move-assign test
// TEST_CASE("MoveAssign", "[tools]") {
//
//   // Set up our vector with a single element, but enough capacity for 4.
//   this->theVector.reserve(4);
//   this->theVector.push_back(Constructable(1));
//
//   // Set up the other vector with 2 elements.
//   this->otherVector.push_back(Constructable(2));
//   this->otherVector.push_back(Constructable(3));
//
//   // Move-assign from the other vector.
//   this->theVector = std::move(this->otherVector);
//
//   // Make sure we have the right result.
//   this->assertValuesInOrder(this->theVector, 2u, 2, 3);
//
//   // Make sure the # of constructor/destructor calls line up. There
//   // are two live objects after clearing the other vector.
//   this->otherVector.clear();
//   REQUIRE(
//     Constructable::getNumConstructorCalls()-2,
//     == Constructable::getNumDestructorCalls()
//   );
//
//   // There shouldn't be any live objects any more.
//   this->theVector.clear();
//   REQUIRE(
//     Constructable::getNumConstructorCalls(),
//     == Constructable::getNumDestructorCalls()
//   );
// }
//
// // Erase a single element
// TEST_CASE("Erase", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//   const auto &theConstVector = this->theVector;
//   this->theVector.erase(theConstVector.begin());
//   this->assertValuesInOrder(this->theVector, 2u, 2, 3);
// }
//
// // Erase a range of elements
// TEST_CASE("EraseRange", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//   const auto &theConstVector = this->theVector;
//   this->theVector.erase(theConstVector.begin(), theConstVector.begin() + 2);
//   this->assertValuesInOrder(this->theVector, 1u, 3);
// }
//
// // Insert a single element.
// TEST_CASE("Insert", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//   typename TypeParam::iterator I =
//     this->theVector.insert(this->theVector.begin() + 1, Constructable(77));
//   REQUIRE(this->theVector.begin() + 1 == I);
//   this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
// }
//
// // Insert a copy of a single element.
// TEST_CASE("Insert", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//   Constructable C(77);
//   typename TypeParam::iterator I =
//       this->theVector.insert(this->theVector.begin() + 1, C);
//   REQUIRE(this->theVector.begin() + 1 == I);
//   this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
// }
//
// // Insert repeated elements.
// TEST_CASE("InsertRepeated", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 4);
//   Constructable::reset();
//   auto I =
//       this->theVector.insert(this->theVector.begin() + 1, 2, Constructable(16));
//   // Move construct the top element into newly allocated space, and optionally
//   // reallocate the whole buffer, move constructing into it.
//   // FIXME: This is inefficient, we shouldn't move things into newly allocated
//   // space, then move them up/around, there should only be 2 or 4 move
//   // constructions here.
//   REQUIRE(Constructable::getNumMoveConstructorCalls() == 2 ||
//               Constructable::getNumMoveConstructorCalls() == 6);
//   // Move assign the next two to shift them up and make a gap.
//   REQUIRE(1 == Constructable::getNumMoveAssignmentCalls());
//   // Copy construct the two new elements from the parameter.
//   REQUIRE(2 == Constructable::getNumCopyAssignmentCalls());
//   // All without any copy construction.
//   REQUIRE(0 == Constructable::getNumCopyConstructorCalls());
//   REQUIRE(this->theVector.begin() + 1 == I);
//   this->assertValuesInOrder(this->theVector, 6u, 1, 16, 16, 2, 3, 4);
// }
//
// TEST_CASE("InsertRepeatedNonIter", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 4);
//   Constructable::reset();
//   auto I = this->theVector.insert(this->theVector.begin() + 1, 2, 7);
//   REQUIRE(this->theVector.begin() + 1 == I);
//   this->assertValuesInOrder(this->theVector, 6u, 1, 7, 7, 2, 3, 4);
// }
//
// TEST_CASE("InsertRepeatedAtEnd", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 4);
//   Constructable::reset();
//   auto I = this->theVector.insert(this->theVector.end(), 2, Constructable(16));
//   // Just copy construct them into newly allocated space
//   REQUIRE(2 == Constructable::getNumCopyConstructorCalls());
//   // Move everything across if reallocation is needed.
//   REQUIRE((
//     Constructable::getNumMoveConstructorCalls() == 0
//     || Constructable::getNumMoveConstructorCalls() == 4
//   ));
//   // Without ever moving or copying anything else.
//   REQUIRE(0 == Constructable::getNumCopyAssignmentCalls());
//   REQUIRE(0 == Constructable::getNumMoveAssignmentCalls());
//
//   REQUIRE(this->theVector.begin() + 4 == I);
//   this->assertValuesInOrder(this->theVector, 6u, 1, 2, 3, 4, 16, 16);
// }
//
// TEST_CASE("InsertRepeatedEmpty", "[tools]") {
//
//   this->makeSequence(this->theVector, 10, 15);
//
//   // Empty insert.
//   REQUIRE(
//     this->theVector.end() ==
//     this->theVector.insert( this->theVector.end(), 0, Constructable(42) )
//   );
//   REQUIRE(
//     this->theVector.begin() + 1 ==
//     this->theVector.insert( this->theVector.begin() + 1, 0, Constructable(42) )
//   );
// }
//
// // Insert range.
// TEST_CASE("InsertRange", "[tools]") {
//
//   Constructable Arr[3] =
//     { Constructable(77), Constructable(77), Constructable(77) };
//
//   this->makeSequence(this->theVector, 1, 3);
//   Constructable::reset();
//   auto I = this->theVector.insert(this->theVector.begin() + 1, Arr, Arr + 3);
//   // Move construct the top 3 elements into newly allocated space.
//   // Possibly move the whole sequence into new space first.
//   // FIXME: This is inefficient, we shouldn't move things into newly allocated
//   // space, then move them up/around, there should only be 2 or 3 move
//   // constructions here.
//   REQUIRE(Constructable::getNumMoveConstructorCalls() == 2 ||
//               Constructable::getNumMoveConstructorCalls() == 5);
//   // Copy assign the lower 2 new elements into existing space.
//   REQUIRE(2 == Constructable::getNumCopyAssignmentCalls());
//   // Copy construct the third element into newly allocated space.
//   REQUIRE(1 == Constructable::getNumCopyConstructorCalls());
//   REQUIRE(this->theVector.begin() + 1 == I);
//   this->assertValuesInOrder(this->theVector, 6u, 1, 77, 77, 77, 2, 3);
// }
//
//
// TEST_CASE("InsertRangeAtEnd", "[tools]") {
//
//   Constructable Arr[3] =
//     { Constructable(77), Constructable(77), Constructable(77) };
//
//   this->makeSequence(this->theVector, 1, 3);
//
//   // Insert at end.
//   Constructable::reset();
//   auto I = this->theVector.insert(this->theVector.end(), Arr, Arr+3);
//   // Copy construct the 3 elements into new space at the top.
//   REQUIRE(3 == Constructable::getNumCopyConstructorCalls());
//   // Don't copy/move anything else.
//   REQUIRE(0 == Constructable::getNumCopyAssignmentCalls());
//   // Reallocation might occur, causing all elements to be moved into the new
//   // buffer.
//   REQUIRE(Constructable::getNumMoveConstructorCalls() == 0 ||
//               Constructable::getNumMoveConstructorCalls() == 3);
//   REQUIRE(0 == Constructable::getNumMoveAssignmentCalls());
//   REQUIRE(this->theVector.begin() + 3 == I);
//   this->assertValuesInOrder(this->theVector, 6u, 1, 2, 3, 77, 77, 77);
// }
//
// TEST_CASE("InsertEmptyRange", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//
//   // Empty insert.
//   REQUIRE(
//     this->theVector.end()
//     == this->theVector.insert(
//       this->theVector.end(),
//       this->theVector.begin(),
//       this->theVector.begin()
//     )
//   );
//   REQUIRE(
//     this->theVector.begin() + 1
//     == this->theVector.insert(
//       this->theVector.begin() + 1,
//       this->theVector.begin(),
//       this->theVector.begin()
//     )
//   );
// }
//
// // Comparison tests.
// TEST_CASE("Comparison", "[tools]") {
//
//   this->makeSequence(this->theVector, 1, 3);
//   this->makeSequence(this->otherVector, 1, 3);
//
//   REQUIRE(this->theVector == this->otherVector);
//   REQUIRE_FALSE(this->theVector != this->otherVector);
//
//   this->otherVector.clear();
//   this->makeSequence(this->otherVector, 2, 4);
//
//   REQUIRE_FALSE(this->theVector == this->otherVector);
//   REQUIRE(this->theVector != this->otherVector);
// }
//
// // Constant vector tests.
// TEST_CASE("ConstVector", "[tools]") {
//   const TypeParam constVector;
//
//   REQUIRE(0u == constVector.size());
//   REQUIRE(constVector.empty());
//   REQUIRE(constVector.begin() == constVector.end());
// }
//
// // Direct array access.
// TEST_CASE("DirectVector", "[tools]") {
//   REQUIRE(0u == this->theVector.size());
//   this->theVector.reserve(4);
//   // EXPECT_LE(4u == this->theVector.capacity()); todo
//   REQUIRE(0 == Constructable::getNumConstructorCalls());
//   this->theVector.push_back(1);
//   this->theVector.push_back(2);
//   this->theVector.push_back(3);
//   this->theVector.push_back(4);
//   REQUIRE(4u == this->theVector.size());
//   REQUIRE(8 == Constructable::getNumConstructorCalls());
//   REQUIRE(1 == this->theVector[0].getValue());
//   REQUIRE(2 == this->theVector[1].getValue());
//   REQUIRE(3 == this->theVector[2].getValue());
//   REQUIRE(4 == this->theVector[3].getValue());
// }
//
// TEST_CASE("Iterator", "[tools]") {
//   std::list<int> L;
//   this->theVector.insert(this->theVector.end(), L.begin(), L.end());
// }
//
// template <typename InvalidType> class DualSmallVectorsTest;
//
// // template <typename VectorT1, typename VectorT2>
// // class DualSmallVectorsTest<std::pair<VectorT1, VectorT2>> : public SmallVectorTestBase {
// // protected:
// //   VectorT1 theVector;
// //   VectorT2 otherVector;
// //
// //   template <typename T, unsigned N>
// //   static unsigned NumBuiltinElts(const SmallVector<T, N>&) { return N; }
// // };
// //
// // typedef ::testing::Types<
// //     // Small mode -> Small mode.
// //     std::pair<SmallVector<Constructable, 4>, SmallVector<Constructable, 4>>,
// //     // Small mode -> Big mode.
// //     std::pair<SmallVector<Constructable, 4>, SmallVector<Constructable, 2>>,
// //     // Big mode -> Small mode.
// //     std::pair<SmallVector<Constructable, 2>, SmallVector<Constructable, 4>>,
// //     // Big mode -> Big mode.
// //     std::pair<SmallVector<Constructable, 2>, SmallVector<Constructable, 2>>
// //   > DualSmallVectorTestTypes;
// //
// // TYPED_TEST_CASE(DualSmallVectorsTest, DualSmallVectorTestTypes);
//
// TEST_CASE("MoveAssign", "[tools]") {
//
//   // Set up our vector with four elements.
//   for (unsigned I = 0; I < 4; ++I)
//     this->otherVector.push_back(Constructable(I));
//
//   const Constructable *OrigDataPtr = this->otherVector.data();
//
//   // Move-assign from the other vector.
//   this->theVector =
//     std::move(static_cast<SmallVectorImpl<Constructable>&>(this->otherVector));
//
//   // Make sure we have the right result.
//   this->assertValuesInOrder(this->theVector, 4u, 0, 1, 2, 3);
//
//   // Make sure the # of constructor/destructor calls line up. There
//   // are two live objects after clearing the other vector.
//   this->otherVector.clear();
//   REQUIRE(
//     Constructable::getNumConstructorCalls()-4
//       == Constructable::getNumDestructorCalls()
//   );
//
//   // If the source vector (otherVector) was in small-mode, assert that we just
//   // moved the data pointer over.
//   REQUIRE(this->NumBuiltinElts(this->otherVector) == 4 ||
//               this->theVector.data() == OrigDataPtr);
//
//   // There shouldn't be any live objects any more.
//   this->theVector.clear();
//   REQUIRE(
//     Constructable::getNumConstructorCalls(),
//       == Constructable::getNumDestructorCalls()
//   );
//
//   // We shouldn't have copied anything in this whole process.
//   REQUIRE( Constructable::getNumCopyConstructorCalls() == 0 );
// }
//
// struct notassignable {
//   int &x;
//   notassignable(int &x) : x(x) {}
// };
//
// TEST(SmallVectorCustomTest, NoAssignTest) {
//   int x = 0;
//   SmallVector<notassignable, 2> vec;
//   vec.push_back(notassignable(x));
//   x = 42;
//   REQUIRE(42 == vec.pop_back_val().x);
// }
//
// struct MovedFrom {
//   bool hasValue;
//   MovedFrom() : hasValue(true) {
//   }
//   MovedFrom(MovedFrom&& m) : hasValue(m.hasValue) {
//     m.hasValue = false;
//   }
//   MovedFrom &operator=(MovedFrom&& m) {
//     hasValue = m.hasValue;
//     m.hasValue = false;
//     return *this;
//   }
// };
//
// TEST_CASE("MidInsert", "[tools]") {
//   SmallVector<MovedFrom, 3> v;
//   v.push_back(MovedFrom());
//   v.insert(v.begin(), MovedFrom());
//   for (MovedFrom &m : v)
//     REQUIRE(m.hasValue);
// }
//
// enum EmplaceableArgState {
//   EAS_Defaulted,
//   EAS_Arg,
//   EAS_LValue,
//   EAS_RValue,
//   EAS_Failure
// };
// template <int I> struct EmplaceableArg {
//   EmplaceableArgState State;
//   EmplaceableArg() : State(EAS_Defaulted) {}
//   EmplaceableArg(EmplaceableArg &&X)
//       : State(X.State == EAS_Arg ? EAS_RValue : EAS_Failure) {}
//   EmplaceableArg(EmplaceableArg &X)
//       : State(X.State == EAS_Arg ? EAS_LValue : EAS_Failure) {}
//
//   explicit EmplaceableArg(bool) : State(EAS_Arg) {}
//
// private:
//   EmplaceableArg &operator=(EmplaceableArg &&) = delete;
//   EmplaceableArg &operator=(const EmplaceableArg &) = delete;
// };
//
// enum EmplaceableState { ES_Emplaced, ES_Moved };
// struct Emplaceable {
//   EmplaceableArg<0> A0;
//   EmplaceableArg<1> A1;
//   EmplaceableArg<2> A2;
//   EmplaceableArg<3> A3;
//   EmplaceableState State;
//
//   Emplaceable() : State(ES_Emplaced) {}
//
//   template <class A0Ty>
//   explicit Emplaceable(A0Ty &&A0)
//       : A0(std::forward<A0Ty>(A0)), State(ES_Emplaced) {}
//
//   template <class A0Ty, class A1Ty>
//   Emplaceable(A0Ty &&A0, A1Ty &&A1)
//       : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
//         State(ES_Emplaced) {}
//
//   template <class A0Ty, class A1Ty, class A2Ty>
//   Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2)
//       : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
//         A2(std::forward<A2Ty>(A2)), State(ES_Emplaced) {}
//
//   template <class A0Ty, class A1Ty, class A2Ty, class A3Ty>
//   Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2, A3Ty &&A3)
//       : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
//         A2(std::forward<A2Ty>(A2)), A3(std::forward<A3Ty>(A3)),
//         State(ES_Emplaced) {}
//
//   Emplaceable(Emplaceable &&) : State(ES_Moved) {}
//   Emplaceable &operator=(Emplaceable &&) {
//     State = ES_Moved;
//     return *this;
//   }
//
// private:
//   Emplaceable(const Emplaceable &) = delete;
//   Emplaceable &operator=(const Emplaceable &) = delete;
// };
//
// TEST_CASE("EmplaceBack", "[tools]") {
//   EmplaceableArg<0> A0(true);
//   EmplaceableArg<1> A1(true);
//   EmplaceableArg<2> A2(true);
//   EmplaceableArg<3> A3(true);
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back();
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_Defaulted);
//     REQUIRE(back.A1.State == EAS_Defaulted);
//     REQUIRE(back.A2.State == EAS_Defaulted);
//     REQUIRE(back.A3.State == EAS_Defaulted);
//   }
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back(std::move(A0));
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_RValue);
//     REQUIRE(back.A1.State == EAS_Defaulted);
//     REQUIRE(back.A2.State == EAS_Defaulted);
//     REQUIRE(back.A3.State == EAS_Defaulted);
//   }
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back(A0);
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_LValue);
//     REQUIRE(back.A1.State == EAS_Defaulted);
//     REQUIRE(back.A2.State == EAS_Defaulted);
//     REQUIRE(back.A3.State == EAS_Defaulted);
//   }
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back(A0, A1);
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_LValue);
//     REQUIRE(back.A1.State == EAS_LValue);
//     REQUIRE(back.A2.State == EAS_Defaulted);
//     REQUIRE(back.A3.State == EAS_Defaulted);
//   }
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back(std::move(A0), std::move(A1));
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_RValue);
//     REQUIRE(back.A1.State == EAS_RValue);
//     REQUIRE(back.A2.State == EAS_Defaulted);
//     REQUIRE(back.A3.State == EAS_Defaulted);
//   }
//   {
//     SmallVector<Emplaceable, 3> V;
//     Emplaceable &back = V.emplace_back(std::move(A0), A1, std::move(A2), A3);
//     REQUIRE(&back == &V.back());
//     REQUIRE(V.size() == 1);
//     REQUIRE(back.State == ES_Emplaced);
//     REQUIRE(back.A0.State == EAS_RValue);
//     REQUIRE(back.A1.State == EAS_LValue);
//     REQUIRE(back.A2.State == EAS_RValue);
//     REQUIRE(back.A3.State == EAS_LValue);
//   }
//   {
//     SmallVector<int, 1> V;
//     V.emplace_back();
//     V.emplace_back(42);
//     REQUIRE(2U == V.size());
//     REQUIRE(0 == V[0]);
//     REQUIRE(42 == V[1]);
//   }
// }
//
// TEST_CASE("InitializerList", "[tools]") {
//   SmallVector<int, 2> V1 = {};
//   REQUIRE(V1.empty());
//   V1 = {0, 0};
//   REQUIRE(makeArrayRef(V1).equals({0, 0}));
//   V1 = {-1, -1};
//   REQUIRE(makeArrayRef(V1).equals({-1, -1}));
//
//   SmallVector<int, 2> V2 = {1, 2, 3, 4};
//   REQUIRE(makeArrayRef(V2).equals({1, 2, 3, 4}));
//   V2.assign({4});
//   REQUIRE(makeArrayRef(V2).equals({4}));
//   V2.append({3, 2});
//   REQUIRE(makeArrayRef(V2).equals({4, 3, 2}));
//   V2.insert(V2.begin() + 1, 5);
//   REQUIRE(makeArrayRef(V2).equals({4, 5, 3, 2}));
// }
//
// } // end namespace


// adapted from https://github.com/dendibakh/prep/blob/master/SmallVector.cpp

template <size_t N>
struct SmallVectorFake : public emp::SmallVector<int, N> {
  // using emp::SmallVector<int, N>::smallBuffer;
  // using emp::SmallVector<int, N>::size;
  // using emp::SmallVector<int, N>::capacity;
  // using emp::SmallVector<int, N>::heapVector;
  using emp::SmallVector<int, N>::isSmall;
};

TEST_CASE("SmallVector, sample", "[tools]") {
  emp::SmallVector<int, 4> vec;
  vec.push_back(0);
  REQUIRE(0 == vec[0]);
}

TEST_CASE("SmallVector, exceeding_small_buffer", "[tools]") {
  SmallVectorFake<2> vec;
  vec.push_back(0);
  vec.push_back(1);
  vec.push_back(5);

  REQUIRE(0 == vec[0]);
  REQUIRE(1 == vec[1]);
  REQUIRE(5 == vec[2]);
  REQUIRE(3 == vec.size());
  REQUIRE(3 <= vec.capacity());
  // we switched to heap allocated buffer
  REQUIRE( ! vec.isSmall() );
  // smallBuffer is still alive
  // REQUIRE(0 == vec.smallBuffer[0]);
  // REQUIRE(1 == vec.smallBuffer[1]);
}

TEST_CASE("SmallVector, not_exceeding_the_stack_buffer", "[tools]") {
  SmallVectorFake<4> vec;
  vec.push_back(0);
  vec.push_back(1);
  vec.push_back(5);

  REQUIRE(0 == vec[0]);
  REQUIRE(1 == vec[1]);
  REQUIRE(5 == vec[2]);
  REQUIRE(3 == vec.size());
  REQUIRE(4 == vec.capacity());
  // we didn't switch to heap allocated buffer
  REQUIRE( vec.isSmall() );
}

TEST_CASE("SmallVector, exceeding_small_buffer_two_times", "[tools]") {

  SmallVectorFake<2> vec;
  vec.push_back(0);
  vec.push_back(1);
  vec.push_back(5);
  vec.push_back(6);
  vec.push_back(7);

  REQUIRE(0 == vec[0]);
  REQUIRE(1 == vec[1]);
  REQUIRE(5 == vec[2]);
  REQUIRE(6 == vec[3]);
  REQUIRE(7 == vec[4]);

  REQUIRE(5 == vec.size());
  REQUIRE(5 <= vec.capacity());

  // we switched to heap allocated buffer
  REQUIRE( !vec.isSmall()  );

  // smallBuffer is still alive
  // REQUIRE(0 == vec.smallBuffer[0]);
  // REQUIRE(1 == vec.smallBuffer[1]);
}

// TEST(SmallVector, at_throws_exceptions)
// {
//   SmallVectorFake<2> vec;
//   vec.push_back(0);
//
//   EXPECT_NO_THROW(vec.at(0));
//   EXPECT_THROW(vec.at(1), std::exception);
//   EXPECT_THROW(vec.at(2), std::exception);
//
//   vec.push_back(1);
//
//   EXPECT_NO_THROW(vec.at(0));
//   EXPECT_NO_THROW(vec.at(1));
//   EXPECT_THROW(vec.at(2), std::exception);
//
//   vec.push_back(5);
//
//   EXPECT_NO_THROW(vec.at(0));
//   EXPECT_NO_THROW(vec.at(1));
//   EXPECT_NO_THROW(vec.at(2));
//   EXPECT_THROW(vec.at(3), std::exception);
// }

TEST_CASE("Small vector, iterators", "[tools]") {
  emp::SmallVector<int, 4> v;
  REQUIRE(v.empty());
  REQUIRE(v.begin() == v.end());
  REQUIRE(v.rbegin() == v.rend());

  v.push_back(0);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  // Forward iterator
  size_t i = 0;
  for (emp::SmallVector<int, 4>::iterator it = v.begin(); it != v.end(); ++it) {
    REQUIRE(*it == v[i]);
    i++;
  }

  // Reverse iterator
  i = v.size()-1;
  for (emp::SmallVector<int, 4>::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) {
    REQUIRE(*it == v[i]);
    i--;
  }

  // Constant iterator
  const emp::SmallVector<int, 5> v2 = {4,3,2,1,0};
  i = 0;
  for (emp::SmallVector<int, 5>::const_iterator it = v2.begin(); it != v2.end(); ++it) {
    REQUIRE(*it == v2[i]);
    i++;
  }

  // Constant reverse iterator
  i = v2.size()-1;
  for (emp::SmallVector<int, 5>::const_reverse_iterator it = v2.rbegin(); it != v2.rend(); ++it) {
    REQUIRE(*it == v2[i]);
    i--;
  }
  //emp::SmallVectorTemplateBase<T, true>uninitialized_copy(v.begin(), v.end(), v2.begin());
  //REQUIRE(v[0]==v2[0]);

}

TEST_CASE("Small vector, pointers", "[tools]") {
    emp::SmallVector<int, 4> v = {0,1,2,3};
    REQUIRE(v.front() == v[0]);
    REQUIRE(v.back() == v[v.size()-1]);

    const emp::SmallVector<int, 5> v2 = {4,3,2,1,0};
    REQUIRE(v2.front() == v2[0]);
    REQUIRE(v2.back() == v2[v2.size()-1]);

    REQUIRE(v.size_in_bytes() < v2.size_in_bytes());
    REQUIRE(v.max_size() > 0);
    REQUIRE(v.capacity_in_bytes() > 0);
}

TEST_CASE("Small vector, constructors", "[tools]") {
    // Adapted from https://en.cppreference.com/w/cpp/container/vector/vector
    emp::SmallVector<std::string, 5> words1 {"the", "frogurt", "is", "also", "cursed"};
    
    // words2 == words1
    emp::SmallVector<std::string, 5> words2(words1.begin(), words1.end());

    // words3 == words1
    emp::SmallVector<std::string, 5> words3(words1);

    // words4 is {"Mo", "Mo", "Mo", "Mo", "Mo"}
    emp::SmallVector<std::string, 5> words4(5, "Mo");

    REQUIRE(words1 == words2);
    REQUIRE(words1 == words3);
    REQUIRE(words4[4] == "Mo");

    // Move Constructors
    emp::SmallVector<std::string, 5> words5((emp::SmallVector<std::string, 5>(words1)));
    const emp::SmallVector<std::string, 5> words6(std::move(words2));
    REQUIRE(words1 == words5);
    REQUIRE(words1 == words6);

    emp::SmallVector<std::string, 5> words7;
    words7 = words1;
    const emp::SmallVector<std::string, 5> words8 = words6;
    REQUIRE(words1 == words7);
    REQUIRE(words6 == words8);

}

TEST_CASE("Small vector, tools", "[tools]") {
  emp::SmallVector<std::string, 3> vec = {"a", "b", "c"};
	std::string sum;
	for(auto it=vec.begin(); it!=vec.end(); it++){
		sum += *it;
	}
	REQUIRE(sum == "abc");

	emp::SmallVector<bool, 0> bvec;
	bvec.resize(1);
	REQUIRE(bvec.size() == 1);
	bvec[0] = true;
	REQUIRE(bvec[0] == true);
	bvec.resize(5,false);
	REQUIRE(bvec[1] == false);

	emp::SmallVector<bool, 4> bvec2 = { true, false, true, false };
	REQUIRE(bvec2.size() == 4);
	bvec2.pop_back();
	REQUIRE(bvec2.size() == 3);
}
