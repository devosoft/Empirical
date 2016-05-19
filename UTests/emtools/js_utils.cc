//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <emscripten.h>
#include <string>
#include <array>

#include "../../config/command_line.h"
#include "../../emtools/init.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/js_utils.h"
#include "../../tools/unit_tests.h"
#include "../../tools/assert.h"

struct JSDataObject;

int main(int argc, char* argv[]) {
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Initialize();

  //Test passing arrays to Javascript
  std::array<int32_t, 3> test_data = {10,30,60};

  JSDataObject test_obj_1;
  test_obj_1.val() = 10;
  test_obj_1.word() = "hi";
  test_obj_1.val2() = 4.4;

  JSDataObject test_obj_2;
  test_obj_2.val() = 40;
  test_obj_2.word() = "hi2";
  test_obj_2.val2() = 11.2;

  std::array<JSDataObject, 2> test_data_2 = {test_obj_1, test_obj_2};

  std::array<std::array<std::array<int, 2>, 1>, 5> horrible_array = {{{{0,0}}, {{0,10}}, {{10,10}}, {{20,20}}, {{30, 30}}}};

  std::array<std::array<JSDataObject, 2>, 2> test_data_4 = {{{test_obj_1, test_obj_2}, {test_obj_2, test_obj_2}}};


  emp::pass_array_to_javascript(test_data);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[0];})
		 , "10");

  emp::pass_array_to_javascript(test_data_2);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[1].val;})
		 , "40");
  EMP_TEST_VALUE(
		 EM_ASM_DOUBLE_V({return emp.__incoming_array[1].val2;})
		 , "11.2");

  emp::pass_array_to_javascript(horrible_array);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[4][0][0];})
		 , "30");

  emp::pass_array_to_javascript(test_data_4);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[1][0].val;})
		 , "40");
  EMP_TEST_VALUE(
		 EM_ASM_DOUBLE_V({return emp.__incoming_array[1][0].val2;})
		 , "11.2");

  //Test passing arrays to C++
  //Test ints
  EM_ASM({emp.__outgoing_array = [5, 1, 3]});
  std::array<int, 3> test_arr_1;
  emp::pass_array_to_cpp(test_arr_1);
  EMP_TEST_VALUE(test_arr_1[0], "5");
  EMP_TEST_VALUE(test_arr_1[1], "1");
  EMP_TEST_VALUE(test_arr_1[2], "3");

  //Test floats
  EM_ASM({emp.__outgoing_array = [5.2, 1.5, 3.1]});
  std::array<float, 3> test_arr_2;
  emp::pass_array_to_cpp(test_arr_2);
  EMP_TEST_VALUE(test_arr_2[0], "5.2");
  EMP_TEST_VALUE(test_arr_2[1], "1.5");
  EMP_TEST_VALUE(test_arr_2[2], "3.1");

  //Test doubles
  EM_ASM({emp.__outgoing_array = [5.2, 1.5, 3.1]});
  std::array<double, 3> test_arr_3;
  emp::pass_array_to_cpp(test_arr_3);
  EMP_TEST_VALUE(test_arr_3[0], "5.2");
  EMP_TEST_VALUE(test_arr_3[1], "1.5");
  EMP_TEST_VALUE(test_arr_3[2], "3.1");

  //Test chars
  EM_ASM({emp.__outgoing_array = ["h", "i", "!"]});
  std::array<char, 3> test_arr_4;
  emp::pass_array_to_cpp(test_arr_4);
  EMP_TEST_VALUE(test_arr_4[0], "h");
  EMP_TEST_VALUE(test_arr_4[1], "i");
  EMP_TEST_VALUE(test_arr_4[2], "!");

  //Test std::strings
  EM_ASM({emp.__outgoing_array = ["jello", "world", "!!"]});
  std::array<std::string, 3> test_arr_5;
  emp::pass_array_to_cpp(test_arr_5);
  EMP_TEST_VALUE(test_arr_5[0], "jello");
  EMP_TEST_VALUE(test_arr_5[1], "world");
  EMP_TEST_VALUE(test_arr_5[2], "!!");

  //Test nested arrays
  EM_ASM({emp.__outgoing_array = [[4,5], [3,1], [7,8]]});
  std::array<std::array<int, 2>, 3> test_arr_6;
  emp::pass_array_to_cpp(test_arr_6);
  EMP_TEST_VALUE(test_arr_6[0][0], "4");
  EMP_TEST_VALUE(test_arr_6[0][1], "5");
  EMP_TEST_VALUE(test_arr_6[1][0], "3");
  EMP_TEST_VALUE(test_arr_6[1][1], "1");
  EMP_TEST_VALUE(test_arr_6[2][0], "7");
  EMP_TEST_VALUE(test_arr_6[2][1], "8");

  //Test more deeply nested arrays
  EM_ASM({emp.__outgoing_array = [[["Sooo", "many"], ["strings", "here"]],
				  [["and", "they're"], ["all", "nested"]],
				  [["in", "this"], ["nested", "array!"]]];});
  std::array<std::array<std::array<std::string, 2>, 2>, 3> test_arr_7;
  emp::pass_array_to_cpp(test_arr_7);
  EMP_TEST_VALUE(test_arr_7[0][0][0], "Sooo");
  EMP_TEST_VALUE(test_arr_7[0][0][1], "many");
  EMP_TEST_VALUE(test_arr_7[0][1][0], "strings");
  EMP_TEST_VALUE(test_arr_7[0][1][1], "here");
  EMP_TEST_VALUE(test_arr_7[1][0][0], "and");
  EMP_TEST_VALUE(test_arr_7[1][0][1], "they're");
  EMP_TEST_VALUE(test_arr_7[1][1][0], "all");
  EMP_TEST_VALUE(test_arr_7[1][1][1], "nested");
  EMP_TEST_VALUE(test_arr_7[2][0][0], "in");
  EMP_TEST_VALUE(test_arr_7[2][0][1], "this");
  EMP_TEST_VALUE(test_arr_7[2][1][0], "nested");
  EMP_TEST_VALUE(test_arr_7[2][1][1], "array!");

}
