/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file wordplay-remove.cpp
 *  @brief Test the removal of letters through an alphabet to see if new words are formed.
 */

#include <iostream>
#include <set>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/datastructs/set_utils.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/math/math.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  if (args.size() != 3) {
    std::cout << "Expected: " << args[0] << " [dict file] [removals]" << std::endl;
    exit(0);
  }

  std::string chars = args[2];

  emp::File file(args[1]);                     // Load the file.
  file.CompressWhitespace();                   // Remove unneeded whitespace.
  std::set<std::string> words = file.AsSet();  // Transfer the file into a set.

  size_t count = 0;
  for (const std::string & word : words) {
    std::string word2 = word;
    emp::remove_chars(word2, chars);
    if (count < 10) std::cout << "Converted '" << word << "' to '" << word2 << "'." << std::endl;
    if (word2.size() > 1 && (word2.size() < word.size()) && emp::Has(words, word2)) {
    // if (word2.size() > 1 && emp::Has(words, word2)) {
      std::cout << word << " -> " << word2 << std::endl;
    }
    count++;
  }
  std::cout << count << " words tested." << std::endl;
}
