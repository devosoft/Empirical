/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file QueueCache.cpp
 */

#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/vector.hpp"
#include "emp/datastructs/QueueCache.hpp"

SCENARIO("Queue Caches store elements", "[datastructs]") {
  GIVEN("a queue cache of a certain capacity") {
    const size_t capacity = 10;
    emp::QueueCache<char, int> qch(capacity);

    // Make sure there are no elements in cache.
    REQUIRE(qch.Size() == 0);

    // Make sure capacity is set.
    REQUIRE(qch.Capacity() == capacity);

    WHEN("elements are stored in it") {
      // Put some things in the cache.
      const auto it_a = qch.Put('a', 0);
      const auto it_b = qch.Put('b', 1);
      const auto it_c = qch.Put('c', 2);

      // Check whether iterators point to the right elements.
      REQUIRE(it_a->first == 'a');
      REQUIRE(it_b->first == 'b');
      REQUIRE(it_c->first == 'c');

      REQUIRE(it_a->second == 0);
      REQUIRE(it_b->second == 1);
      REQUIRE(it_c->second == 2);

      THEN("they can be retrieved") {
        // Test getting elements from cache.
        REQUIRE(qch.Get('a') == 0);
        REQUIRE(qch.Get('b') == 1);
        REQUIRE(qch.Get('c') == 2);
      }

      AND_THEN("they can be modified") {
        // Make sure returning a reference works
        qch.Get('a') = 1;
        REQUIRE(qch.Get('a') == 1);
      }

      AND_THEN("they can be overwritten") {
        qch.Put('a', 2);
        REQUIRE(qch.Get('a') == 2);
      }
    }
    WHEN("an element is not in it") {
      AND_WHEN("a function isn't passed") {
        // Test whether exception gets thrown when key is not in cache.
        REQUIRE_THROWS_WITH(qch.Get('d'), "Key not in cache.");
      }
      AND_WHEN("a function is passed") {
        THEN("it is called") {
          // Test whether function gets called correctly when key is not found.
          REQUIRE(qch.Get(
            'd',
            [](char ch){ return (int)ch - 'a'; }
          ) == 3);
        }
      }
    }
  }
}

SCENARIO("Queue Caches can be sized and resized", "[datastructs]") {
  GIVEN("a queue cache of a set size with elements in it") {
    emp::QueueCache<char, int, 10> qch;

    // Put some things in the cache.
    qch.Put('a', 0);
    qch.Put('b', 1);
    qch.Put('c', 2);
    qch.Put('d', 2);

    WHEN("it is shrunk") {
      // first, we get the keys we want to keep. this pushes them to the front of the cache.
      qch.Get('a');
      qch.Get('b');
      // then, we resize the cache.
      qch.SetCapacity(2);

      THEN("its capacity changes") {
        REQUIRE(qch.Capacity() == 2);
      }

      AND_THEN("elements past its new capacity are removed")  {
        // make sure the cache shrunk appropiately.
        REQUIRE(qch.Size() == 2);
        REQUIRE(qch.Get('a') == 0);
        REQUIRE(qch.Get('b') == 1);

        REQUIRE_THROWS_WITH(qch.Get('c'), "Key not in cache.");
        REQUIRE_THROWS_WITH(qch.Get('d'), "Key not in cache.");
      }
    }
  }
}

SCENARIO("Queue Caches behave like queues (first-in, first-out)", "[datastructs]") {
  GIVEN("a queue cache full with elements") {
    emp::QueueCache<char, int, 2> qch;

    // Saturate the cache.
    qch.Put('b', 1);
    qch.Put('a', 0);

    WHEN("another element is added") {
      qch.Put('x', 25);
      THEN("the element is in the cache") {
        REQUIRE(qch.Get('x') == 25);
        REQUIRE(qch.Get('a') == 0);
      }
      AND_THEN("the last element is not") {
        REQUIRE_THROWS_WITH(qch.Get('b'), "Key not in cache.");
      }
    }
  }
}
SCENARIO("Queue Caches can delete elements", "[datastructs]") {
  GIVEN("a queue cache with elements") {
    emp::QueueCache<char, int, 2> qch;

    qch.Put('a', 0);
    qch.Put('b', 1);

    WHEN("we call delete on one of them") {
      qch.Delete('a');

      THEN("it is deleted") {
        REQUIRE_THROWS_WITH(qch.Get('a'), "Key not in cache.");
      }
    }
  }
}

SCENARIO("Queue Caches can be cleared", "[datastructs]") {
  GIVEN("a queue cache with elements") {
    const size_t capacity = 2;
    emp::QueueCache<char, int> qch(capacity);

    qch.Put('a', 0);
    qch.Put('b', 1);

    WHEN("it is cleared") {
      qch.Clear();
      THEN("its elements are removed") {
        REQUIRE(qch.Size() == 0);

        // different ways to check for existence in cache
        REQUIRE_THROWS_WITH(qch.Get('a'), "Key not in cache.");
        REQUIRE(!qch.Contains('b'));
      }
      AND_THEN("its capacity is unchanged") {
        REQUIRE(qch.Capacity() == capacity);
      }
    }
  }
}

SCENARIO("Queue Caches can be subscripted", "[datastructs]") {
  // Test whether the subscript operator can get values from cache.
  GIVEN("a queue cache") {
    emp::QueueCache<char, int, 3> qch;

    const auto it_x = qch.Put('x', 23);
    const auto it_y = qch.Put('y', 24);
    const auto it_z = qch.Put('z', 25);

    WHEN("we subscript into an existing key") {
      const int res_x = qch['x'];
      const int res_y = qch['y'];
      const int res_z = qch['z'];

      THEN("the value from its given key is returned") {
        REQUIRE(res_x == it_x->second);
        REQUIRE(res_y == it_y->second);
        REQUIRE(res_z == it_z->second);
      }
    }
    WHEN("we assign a value to the returned reference") {
      qch['x'] = 1;
      qch['y'] = 2;
      qch['z'] = 3;

      THEN("the value is changed in the cache") {
        REQUIRE(qch.Get('x') == 1);
        REQUIRE(qch.Get('y') == 2);
        REQUIRE(qch.Get('z') == 3);
      }
      AND_THEN("no iterator is invalidated") {
        // Make sure iterators weren't invalidated.
        REQUIRE(qch['x'] == it_x->second);
        REQUIRE(qch['y'] == it_y->second);
        REQUIRE(qch['z'] == it_z->second);
      }
    }
    WHEN("we subscript into a new key") {
      qch['a'] = 64;

      THEN("the element is put into the cache") {
        REQUIRE(qch.Get('a') == 64);
      }
    }
  }
}

SCENARIO("Queue Caches can be iterated on", "[datastructs]") {
  GIVEN("a queue cache with elements in it") {
    emp::QueueCache<char, int, 6> qch;

    qch.Put('A', 'A');
    qch.Put('B', 'B');
    qch.Put('C', 'C');
    qch.Put('D', 'D');
    qch.Put('E', 'E');

    WHEN("we iterate on it with for loops") {
      emp::vector<int> result;
      for (char i = 'A'; i < 'F'; ++i) {
        result.push_back(qch.Get(i));
      }

      THEN("we get the expected result") {
        const emp::vector<int> expected{'A', 'B', 'C', 'D', 'E'};
        CHECK(result == expected);
      }
    }
    WHEN("we apply std::algoritms using begin() and end() iterators") {
      emp::vector<int> result;
      const size_t distance = std::distance(
        qch.begin(),
        qch.end()
      );

      std::transform(
        qch.cbegin(),
        qch.cend(),
        std::back_inserter(result),
        [](const auto& a) {
          return a.first;
        }
      );

      THEN("we get the expected result") {
        const emp::vector<int> expected{'E', 'D', 'C', 'B', 'A'};
        REQUIRE(expected == result);
        CHECK(distance == qch.Size());
      }
    }
  }
}

struct MyInt {
  int val;
  MyInt() = delete;
  MyInt(const int _val) : val(_val) { ; }

  bool operator==(const MyInt& other) const {
    return val == other.val;
  }
};
// operator<< is needed to get Catch2 expansions to work correctly
std::ostream& operator<<(std::ostream& os, const MyInt& in ) {
  os << in.val;
  return os;
}
SCENARIO("Queue Caches can contain non default-constructible values"){
  GIVEN("a queue cache templated on a struct with no default constructor") {
    emp::QueueCache<
      char,
      MyInt,
      2
    > qch;
    WHEN("we attempt to put elements in it") {
      qch.Put('a', 1);
      qch.Put('b', 2);
      THEN("these are stored") {
        REQUIRE(qch.Get('a') == MyInt(1));
        REQUIRE(qch.Contains('b'));
      }
    }
  }
}
