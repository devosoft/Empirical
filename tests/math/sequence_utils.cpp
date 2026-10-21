/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/sequence_utils.hpp"

TEST_CASE("Test sequence utils", "[math]")
{
  // --- Test conversion of strings to sequences ---

  std::string seq_string1 = "17.5";
  emp::vector<double> out_seq = emp::ToSequence<double>(seq_string1);
  CHECK(out_seq.size() == 1);
  CHECK(out_seq[0] == 17.5);

  out_seq = emp::ToSequence<double>("");
  CHECK(out_seq.size() == 0);

  out_seq = emp::ToSequence<double>("1.5, 3.25, 101.125");
  CHECK(out_seq.size() == 3);
  CHECK(out_seq[0] == 1.5);
  CHECK(out_seq[1] == 3.25);
  CHECK(out_seq[2] == 101.125);

  out_seq = emp::ToSequence<double>("1.5:0.5:4, 3.25, 101.125"); // 1.5, 2.0, 2.5, 3.0, 3.5, 3.25, 101.125
  CHECK(out_seq.size() == 7);
  CHECK(out_seq[0] == 1.5);
  CHECK(out_seq[1] == 2.0);
  CHECK(out_seq[4] == 3.5);
  CHECK(out_seq[5] == 3.25);
  CHECK(out_seq[6] == 101.125);



  // --- Test string distances ---
  std::string s1 = "This is the first test string.";
  std::string s2 = "This is the second test string.";

  CHECK(emp::calc_hamming_distance(s1,s2) == 19);
  CHECK(emp::calc_edit_distance(s1,s2) == 6);

  // std::string s3 = "abcdefghijklmnopqrstuvwWxyz";
  // std::string s4 = "abBcdefghijXXmnopqrstuvwxyz";

  // std::string s3 = "lmnopqrstuv";
  // std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  std::string s3 = "adhlmnopqrstuvxy";
  std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  emp::align(s3, s4, '_');

  CHECK(s3 == "a__d___h___lmnopqrstuv_xy_");

  emp::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  emp::vector<int> v2 = { 1,4,5,6,8 };

  emp::align(v1,v2,0);

  CHECK((v2 == emp::vector<int>({1,0,0,4,5,6,0,8,0})));
}

// Local settings for Empecable file checker.
// empecable_words: adhlmnopqrstuvxy lmnopqrstuv xmnopqrstuvwxyz bcdefghij wxyz abcdefghijklmnopqrstuvw

TEST_CASE("Trimmed edit distance matches a full matrix", "[math]") {
  const auto reference = [](const std::string & a, const std::string & b) {
    emp::vector<emp::vector<size_t>> matrix(a.size() + 1, emp::vector<size_t>(b.size() + 1));
    for (size_t i = 0; i <= a.size(); ++i) matrix[i][0] = i;
    for (size_t j = 0; j <= b.size(); ++j) matrix[0][j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
      for (size_t j = 1; j <= b.size(); ++j) {
        matrix[i][j] = std::min({matrix[i-1][j] + 1, matrix[i][j-1] + 1,
                                matrix[i-1][j-1] + (a[i-1] != b[j-1])});
      }
    }
    return matrix.back().back();
  };
  emp::vector<std::string> words{""};
  for (size_t length = 1; length <= 5; ++length) {
    for (size_t bits = 0; bits < (size_t{1} << length); ++bits) {
      std::string word(length, 'a');
      for (size_t i = 0; i < length; ++i) if (bits & (size_t{1} << i)) word[i] = 'b';
      words.push_back(word);
    }
  }
  for (const auto & a : words) for (const auto & b : words) {
    CHECK(emp::calc_edit_distance(a, b) == reference(a, b));
  }
  CHECK(emp::calc_edit_distance(std::string(10000, 'x') + "a" + std::string(10000, 'z'),
                                std::string(10000, 'x') + "b" + std::string(10000, 'z')) == 1);
  CHECK(emp::calc_edit_distance(emp::vector<int>{1, 2, 3}, emp::vector<int>{1, 3}) == 1);
  CHECK(emp::calc_edit_distance(std::vector<bool>{true, false}, std::vector<bool>{true}) == 1);
}
