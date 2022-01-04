/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file char_utils.cpp
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/char_utils.hpp"

TEST_CASE("Test char_utils", "[tools]")
{
	CHECK(emp::is_idchar('a'));
	CHECK(emp::is_idchar('_'));
	CHECK(emp::is_idchar('5'));
	CHECK(!emp::is_idchar('?'));

	CHECK(emp::is_one_of('v',"uvwxyz"));
	CHECK(emp::is_one_of(';',"!?.,;:'()"));
	CHECK(!emp::is_one_of('a',"!?.,;:'()"));

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"_lines_\".";

  // TEST3: Make sure that we can properly identify different types of characters.
  int num_ws = 0;
  int num_cap = 0;
  int num_lower = 0;
  int num_let = 0;
  int num_num = 0;
  int num_alphanum = 0;
  int num_idchar = 0;
  int num_i = 0;
  int num_vowel = 0;

  for (char cur_char : special_string) {
    if (emp::is_whitespace(cur_char)) num_ws++;
    if (emp::is_upper_letter(cur_char)) num_cap++;
    if (emp::is_lower_letter(cur_char)) num_lower++;
    if (emp::is_letter(cur_char)) num_let++;
    if (emp::is_digit(cur_char)) num_num++;
    if (emp::is_alphanumeric(cur_char)) num_alphanum++;
    if (emp::is_idchar(cur_char)) num_idchar++;
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

  CHECK(num_ws == 6);
  CHECK(num_cap == 6);
  CHECK(num_lower == 16);
  CHECK(num_let == 22);
  CHECK(num_num == 3);
  CHECK(num_alphanum == 25);
  CHECK(num_idchar == 27);
  CHECK(num_other == 7);
  CHECK(num_i == 3);
  CHECK(num_vowel == 7);
}


TEST_CASE("Test CharSet", "[tools]")
{
  emp::CharSet cs;
  cs.SetRange('e','p');
  cs.ClearRange('h','j');
  // cs has: e,f,g,k,l,m,n,o,p

  size_t matches = cs.CountMatches("qwertyuiopasdfghjklzxcvbnm");
  CHECK(matches == 9);

  matches = cs.CountMatches("0123456789");
  CHECK(matches == 0);

  CHECK(cs.Has("flop"));
  CHECK(cs.Has("loom"));
  CHECK(cs.Has("poke"));
  CHECK(!cs.Has("most"));
  CHECK(!cs.Has("other"));
  CHECK(!cs.Has("words"));

  CHECK(cs.HasAny("none"));
  CHECK(cs.HasAny("most"));
  CHECK(!cs.HasAny("twitch"));
  CHECK(!cs.HasAny("sarcastic"));
}
