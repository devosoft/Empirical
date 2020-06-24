#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/QueueCache.h"
#include "base/vector.h"

#include <sstream>
#include <string>

SCENARIO("Queue Caches store elements", "[tools]") {
  GIVEN("a queue cache of a certain capacity") {
    const size_t capacity = 10;
    emp::QueueCache<char, int> qch(capacity);

    // Make sure there are no elements in cache.
    REQUIRE(qch.Size() == 0);

    // Make sure capacity is set.
    REQUIRE(qch.Capacity() == capacity);

    WHEN("elements are stored in it") {
      // Put some things in the cache.
      auto it_a = qch.Put('a', 0);
      auto it_b = qch.Put('b', 1);
      auto it_c = qch.Put('c', 2);

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
          std::function<int(char)> qch_fn = [](char ch){ return (int)ch - 'a'; };
          REQUIRE(qch.Get('d', qch_fn) == 3);
        }
      }
    }
  }
}

SCENARIO("Queue Caches can be sized and resized", "[tools]") {
  GIVEN("a queue cache of a set size with elements in it") {
    const size_t capacity = 10;
    emp::QueueCache<char, int> qch(capacity);

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

SCENARIO("Queue Caches behave like queues (first-in, first-out)", "[tools]") {
  GIVEN("a queue cache full with elements") {
    const size_t capacity = 2;
    emp::QueueCache<char, int> qch(capacity);

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
SCENARIO("Queue Caches can delete elements", "[tools]") {
  GIVEN("a queue cache with elements") {
    const size_t capacity = 2;
    emp::QueueCache<char, int> qch(capacity);

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

SCENARIO("Queue Caches can be cleared", "[tools]") {
  GIVEN("a queue cache with elements") {
    const size_t capacity = 2;
    emp::QueueCache<char, int> qch(capacity);

    qch.Put('a', 0);
    qch.Put('b', 1);

    WHEN("it is cleared") {
      qch.Clear();
      THEN("its elements are removed") {
        REQUIRE(qch.Size() == 0);

        REQUIRE_THROWS_WITH(qch.Get('a'), "Key not in cache.");
        REQUIRE_THROWS_WITH(qch.Get('b'), "Key not in cache.");
      }
      AND_THEN("its capacity is unchanged") {
        REQUIRE(qch.Capacity() == capacity);
      }
    }
  }
}

SCENARIO("Queue Caches can be subscripted", "[tools]") {
  // Test whether the subscript operator can get values from cache.
  GIVEN("a queue cache with elements") {
    const size_t capacity = 3;
    emp::QueueCache<char, int> qch(capacity);

    auto it_x = qch.Put('x', 23);
    auto it_y = qch.Put('y', 24);
    auto it_z = qch.Put('z', 25);

    WHEN("we subscript into it") {
      auto res_x = qch['x'];
      auto res_y = qch['y'];
      auto res_z = qch['z'];

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
  }
}

SCENARIO("Queue Caches can be iterated on", "[tools]") {
  GIVEN("a queue cache with elements in it") {
    const size_t capacity = 6;
    emp::QueueCache<char, int> qch(capacity);

    for (char i = 'A'; i < 'F'; ++i) {
      qch.Put(i, i);
    }

    WHEN("we iterate on it") {
      emp::vector<int> result;
      emp::vector<int> expected{'A', 'B', 'C', 'D', 'E'};

      for (char i = 'A'; i < 'F'; ++i) {
        result.push_back(qch.Get(i));
      }

      THEN("we get the expected result") {
        CHECK(result == expected);
      }
    }
    WHEN("we apply std::algoritms using begin() and end() iterators") {
      emp::vector<int> result;
      emp::vector<int> expected{'E', 'D', 'C', 'B', 'A'};

      std::transform(
        qch.cbegin(),
        qch.cend(),
        std::back_inserter(result),
        [](const auto& a) {
          return a.first;
        }
      );

      THEN("we get the expected result") {
        REQUIRE(expected == result);
      }
    }
  }
}
