/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file wordplay-shuffle.cpp
 *  @brief Find words that are a shuffle of another word.
 */

#include <iostream>
#include <set>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/config/ArgManager.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/datastructs/set_utils.hpp"
#include "../../../include/emp/datastructs/vector_utils.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/math/math.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

int main(int argc, char* argv[])
{
  emp::cl::ArgManager args(argc, argv);

  if (args.size() < 3) {
    std::cout << "Expected: " << args[0] << " [dict file] [shuffle positions ...]" << std::endl;
    exit(1);
  }

  size_t from_size = 0;    // Default from size is unbound (marked as 0)
  args.UseArg("-from_size", from_size, "Set the size of the words to be converted from.");

  // Setup the words file.
  emp::File file(args[1]);                     // Load the file.
  file.CompressWhitespace();                   // Remove unneeded whitespace.
  std::set<std::string> words = file.AsSet();  // Transfer the file into a set.

  // Setup the positions
  args.erase(args.begin(), args.begin() + 2);                 // Remove first two args.
  emp::vector<size_t> posv = emp::from_strings<size_t>(args); // Remaining args are positions.

  // Determine minimum word lengths
  size_t min_from_size = posv[FindMaxIndex(posv)] + 1;  // Must be able to index all positions.
  size_t max_from_size = (size_t) -1;                   // By default, no limit on max_from_size.
  int to_size = posv.size();                            // All to positions are specified.
  if (from_size && min_from_size > from_size) {
    std::cout << "Position value " << min_from_size - 1
              << " too high for string of length " << from_size
              << std::endl;
    exit(2);
  }
  if (from_size) {
    min_from_size = from_size;
    max_from_size = from_size;
  }

  size_t test_count = 0;                        // Count the number of words to be tested.
  size_t okay_count = 0;                        // Count the number of words found in dict!.
  std::string word2(to_size, ' ');              // Setup the to word of a given size.
  for (const std::string & word : words) {      // Loop through all words.
    if (word.size() < min_from_size) continue;  // Reject words that are too short.
    if (word.size() > max_from_size) continue;  // Reject words that are too long.

    for (size_t i = 0; i < to_size; i++) {
      word2[i] = word[ posv[i] ];
    }

    if (test_count < 10) {
      std::cout << "Converted '" << word << "' to '" << word2 << "'." << std::endl;
    }
    if (emp::Has(words, word2)) {
      std::cout << word << " -> " << word2 << std::endl;
      okay_count++;
    }
    test_count++;
  }
  std::cout << test_count << " words tested of legal length." << std::endl;
  std::cout << okay_count << " words tested of potential match." << std::endl;
}
