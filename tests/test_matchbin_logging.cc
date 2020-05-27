//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the tools/ folder.


#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

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

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "data/DataNode.h"

#include "tools/File.h"

#include "tools/BitMatrix.h"
#include "tools/BitSet.h"
#include "tools/BitVector.h"
#include "tools/ContiguousStream.h"
#include "tools/Distribution.h"
#include "tools/DFA.h"
#include "tools/DynamicString.h"
#include "tools/FunctionSet.h"
#include "tools/Graph.h"
#include "tools/IndexMap.h"
#include "tools/Lexer.h"

#define EMP_LOG_MATCHBIN
#include "tools/MatchBin.h"


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
#include "tools/keyname_utils.h"
#include "tools/lexer_utils.h"
#include "tools/map_utils.h"
#include "tools/math.h"
#include "tools/matchbin_utils.h"
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


TEST_CASE("Test MatchBin", "[tools]")
{
  // <------- TEST LOGGING ------->
  // test static instance ID
  {
    using matchbin_t = emp::MatchBin<
      std::string,
      emp::StreakMetric<64>,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
   >;

    emp::vector<matchbin_t> matchbins;
    std::set<size_t> ids;
    emp::Random rand(1);

    // create vector of matchbins
    for (size_t i = 0; i < 100; ++i) {
      matchbin_t bin(rand);
      matchbins.push_back(bin);
      bin.log.Activate();
    }
    // test that every matchbin has a unique ID
    for (auto& bin: matchbins) {
      ids.insert(bin.log.GetID());
      bin.log.FlushLogBuffer();
    }

    REQUIRE(ids.size() == matchbins.size());
  }
  {
  emp::Random rand(1);
  emp::MatchBin<
      std::string,
      emp::StreakMetric<64>,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
  > bin(rand);
  // is the macro setting the constexpr?
  REQUIRE(bin.log.IsEnabled());

  // logging should be deactivated by default even when macro is set
  REQUIRE(!bin.log.IsActivated());

  // are we actually activating logging?
  bin.log.Activate();
  REQUIRE(bin.log.IsActivated());

  // can we deactivate it?
  bin.log.Deactivate();
  REQUIRE(!bin.log.IsActivated());

  // what about setting it to a bool?
  bin.log.Set(true);
  REQUIRE(bin.log.IsActivated());
  }
  // test EmplaceDataFile
  {
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    > bin(rand);
    bin.log.Activate();

    // output to a file
    bin.log.EmplaceDataFile("datafile.csv");

    // put some data in the matchbin
    bin.Put("1", 1);
    bin.Put("2", 2);

    // do some matches
    bin.Match(2);
    bin.MatchRaw(2);

    bin.Match(2);
    bin.MatchRaw(2);

    std::cout << "Printing logging test:" << std::endl;
    // emplace an ostream without flushing
    bin.log.EmplaceDataFile(std::cout);

    bin.MatchRaw(2);
    bin.Match(2);

    bin.log.FlushLogBuffer();
  }
  // test writing to an emp::File
  {
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    > bin(rand);
    bin.log.Activate();

    // log to a string stream
    std::stringstream ss;
    bin.log.EmplaceDataFile(ss);

    // put some data in the matchbin
    for (size_t i = 0; i < 50; ++i) {
      bin.Put(emp::to_string(i), i);
    }

    // do matches
    for (size_t i = 0; i < 50; ++i) {
      bin.Match(i);
      bin.MatchRaw(i);
    }

    bin.log.WriteLogBuffer();

    // make an emp::File from our string stream
    emp::File file(ss);

    // read from the emp::File
    emp::vector<emp::vector<std::string>> data = file.ToData<std::string>();

    // check that every line is unique
    std::set<emp::vector<std::string>> dataset;
    for (size_t i = 0; i < data.size(); ++i) dataset.insert(data[i]);

    REQUIRE(dataset.size() == data.size());

    bin.log.FlushLogBuffer();
  }
  // test clearing the logbuffer in various ways
  {
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    > bin(rand);
    bin.log.Activate();

    std::stringstream ss;
    bin.log.EmplaceDataFile(ss);

    bin.Put("1", 1);
    bin.Put("2", 2);

    bin.Match(2);
    bin.MatchRaw(2);

    // test clear log buffer
    bin.log.ClearLogBuffer();

    REQUIRE(bin.log.GetLogBuffer().empty());

    // write an empty file
    bin.log.WriteLogBuffer();

    emp::File file(ss);

    auto data = file.ToData<std::string>();

    // buffer should be empty, so must only have a header and an EOF
    REQUIRE(data.size() == 2);

    bin.Match(1);
    bin.MatchRaw(1);

    // test flush log buffer
    bin.log.FlushLogBuffer();

    data = file.ToData<std::string>();

    REQUIRE(data.size() == 2);
    REQUIRE(bin.log.GetLogBuffer().empty());
  }
  // test whether we write the correct number of lines with a single matchbin
  {
    emp::Random rand(1);
    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    > bin(rand);
    bin.log.Activate();

    std::stringstream ss;
    bin.log.EmplaceDataFile(ss);

    bin.Put("1", 1);
    bin.Put("2", 2);

    bin.Match(2);
    bin.MatchRaw(2);

    // test flush log buffer
    bin.log.FlushLogBuffer();

    emp::File file(ss);

    auto data = file.ToData<std::string>();

    /*
     there have to be four lines written to our file:
      * 1 header
      * 2 matches
      * 1 EOF newline
    */
    REQUIRE(data.size() == 4);
    REQUIRE(bin.log.GetLogBuffer().empty());
  }
  // test whether multiple matchbins can write to the same ostream concurrently
  {
    // number of matchbins to test
    const size_t n = 37;

    emp::Random rand(1);
    using matchbin_t = emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RankedSelector<std::ratio<1+1, 1>>,
      emp::LegacyRegulator
    >;

    std::vector<matchbin_t> matchbins;

    // create n matchbins
    for (size_t i = 0; i < n; ++i) {
      matchbins.emplace_back(rand);
    }

    // match for all matchbins
    std::stringstream ss;
    for (auto& bin: matchbins) {
      bin.log.Activate();
      bin.log.EmplaceDataFile(ss);
      bin.Put("1", 1);
      bin.Put("2", 2);

      bin.Match(2);
      bin.MatchRaw(2);
    }

    // flush all log buffers
    for (auto& bin: matchbins) {
      bin.log.FlushLogBuffer();
    }

    emp::File file(ss);

    auto data = file.ToData<std::string>();

    // data must be equal to 3n + 1, given that for each matchbin we have
    // 1 header and 2 matches, and at the end an EOF newline
    REQUIRE(data.size() == 3 * n + 1);
  }
}
#undef EMP_LOG_MATCHBIN