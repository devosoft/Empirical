//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include <string>
#include <array>

#include "../../tests2/unit_tests.h"
#include "config/command_line.h"
#include "base/assert.h"
#include "base/vector.h"
#include "web/init.h"
#include "web/JSWrap.h"
#include "web/js_utils.h"

struct JSDataObject {
  EMP_BUILD_INTROSPECTIVE_TUPLE(
        int, val,
        std::string, word,
        double, val2
    )
};

int main(int argc, char* argv[]) {
  //emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  //const bool verbose = emp::cl::use_arg(args, "-v");
  [[maybe_unused]] bool verbose = true;

  emp::Initialize();

  //Test passing arrays to Javascript
  emp::array<int32_t, 3> test_data = {{10,30,60}};

  JSDataObject test_obj_1;
  test_obj_1.val() = 10;
  test_obj_1.word() = "hi";
  test_obj_1.val2() = 4.4;

  JSDataObject test_obj_2;
  test_obj_2.val() = 40;
  test_obj_2.word() = "hi2";
  test_obj_2.val2() = 11.2;

  emp::array<JSDataObject, 2> test_data_2 = {{test_obj_1, test_obj_2}};

  emp::array<emp::array<emp::array<int, 2>, 1>, 5> horrible_array = {{{{{{0,0}}}}, {{{{0,10}}}}, {{{{10,10}}}}, {{{{20,20}}}}, {{{{30, 30}}}}}};

  emp::array<emp::array<JSDataObject, 2>, 2> test_data_4 = {{{{test_obj_1, test_obj_2}}, {{test_obj_2, test_obj_2}}}};

  emp::array<emp::array<std::string, 5>, 1> string_arr = {{{{"do", "strings", "work", "in", "arrays?"}}}};

  emp::vector<int> int_vec = {5,1,2,3,6};

  emp::vector<std::string> string_vec = {"a", "vector", "of", "strings"};

  emp::vector<emp::vector<emp::vector<double> > > nested_vec = {{{1,2,3},{4,5,6}}};

  emp::pass_array_to_javascript(nested_vec);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp_i.__incoming_array[0][1][1];})
		 , 5);

  emp::pass_array_to_javascript(int_vec);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp_i.__incoming_array[4];})
		 , 6);

  // @CAO: Note that the code below leaks memeory because the malloc's memory is never freed.
  emp::pass_array_to_javascript(string_vec);
  EMP_TEST_VALUE(
		 emp::to_string( (char *) EM_ASM_INT_V({
           var alloc_size = emp_i.__incoming_array[1].length*2;
		       var buffer = Module._malloc(alloc_size);
		       stringToUTF8(emp_i.__incoming_array[1], buffer, alloc_size);
		       return buffer;
		     }) ),
		 "vector");

  emp::pass_array_to_javascript(test_data);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp_i.__incoming_array[0];})
		 , 10);

  emp::pass_array_to_javascript(test_data_2);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp_i.__incoming_array[1].val;})
		 , 40);
  EMP_TEST_VALUE(
		 EM_ASM_DOUBLE_V({return emp_i.__incoming_array[1].val2;})
		 , 11.2);

  // @CAO: Note that the code below leaks memeory because the malloc's memory is never freed.
  emp::pass_array_to_javascript(string_arr);
  EMP_TEST_VALUE(
		 emp::to_string( (char *) EM_ASM_INT_V({
           var alloc_size = emp_i.__incoming_array[0][3].length*2;
		       var buffer = Module._malloc(alloc_size);
		       stringToUTF8(emp_i.__incoming_array[0][3], buffer, alloc_size);
		       return buffer;
		     }) ),
		 "in");

  emp::pass_array_to_javascript(horrible_array);
  EMP_TEST_VALUE(EM_ASM_INT_V({return emp_i.__incoming_array[4][0][0];}), 30);

  emp::pass_array_to_javascript(test_data_4);
  EMP_TEST_VALUE( EM_ASM_INT_V({return emp_i.__incoming_array[1][0].val;}), 40);
  EMP_TEST_VALUE( EM_ASM_DOUBLE_V({return emp_i.__incoming_array[1][0].val2;}), 11.2);

  //Test passing arrays to C++
  //Test ints
  EM_ASM({emp_i.__outgoing_array = ([5, 1, 3])});
  emp::array<int, 3> test_arr_1;
  emp::pass_array_to_cpp(test_arr_1);
  EMP_TEST_VALUE(test_arr_1[0], 5);
  EMP_TEST_VALUE(test_arr_1[1], 1);
  EMP_TEST_VALUE(test_arr_1[2], 3);

  //Test floats
  EM_ASM({emp_i.__outgoing_array = ([5.2, 1.5, 3.1])});
  emp::array<float, 3> test_arr_2;
  emp::pass_array_to_cpp(test_arr_2);
  EMP_TEST_VALUE(emp::to_string(test_arr_2[0]), emp::to_string(5.2));
  EMP_TEST_VALUE(test_arr_2[1], 1.5);
  EMP_TEST_VALUE(emp::to_string(test_arr_2[2]), emp::to_string(3.1));

  //Test doubles
  EM_ASM({emp_i.__outgoing_array = ([5.2, 1.5, 3.1])});
  emp::array<double, 3> test_arr_3;
  emp::pass_array_to_cpp(test_arr_3);
  EMP_TEST_VALUE(test_arr_3[0], 5.2);
  EMP_TEST_VALUE(test_arr_3[1], 1.5);
  EMP_TEST_VALUE(test_arr_3[2], 3.1);

  //Test doubles in vector
  EM_ASM({emp_i.__outgoing_array = ([5.3, 1.6, 3.2])});
  emp::vector<double> test_vec;
  emp::pass_vector_to_cpp(test_vec);
  EMP_TEST_VALUE(test_vec[0], 5.3);
  EMP_TEST_VALUE(test_vec[1], 1.6);
  EMP_TEST_VALUE(test_vec[2], 3.2);

  //Test chars
  EM_ASM({emp_i.__outgoing_array = (["h", "i", "!"])});
  emp::array<char, 3> test_arr_4;
  emp::pass_array_to_cpp(test_arr_4);
  EMP_TEST_VALUE(test_arr_4[0], 'h');
  EMP_TEST_VALUE(test_arr_4[1], 'i');
  EMP_TEST_VALUE(test_arr_4[2], '!');
  emp::vector<char> test_vec_4;
  emp::pass_vector_to_cpp(test_vec_4);
  EMP_TEST_VALUE(test_vec_4[0], 'h');
  EMP_TEST_VALUE(test_vec_4[1], 'i');
  EMP_TEST_VALUE(test_vec_4[2], '!');


  //Test std::strings
  EM_ASM({emp_i.__outgoing_array = (["jello", "world", "!!"])});
  emp::array<std::string, 3> test_arr_5;
  emp::pass_array_to_cpp(test_arr_5);
  EMP_TEST_VALUE(test_arr_5[0], "jello");
  EMP_TEST_VALUE(test_arr_5[1], "world");
  EMP_TEST_VALUE(test_arr_5[2], "!!");
  emp::vector<std::string> test_vec_5;
  emp::pass_vector_to_cpp(test_vec_5);
  EMP_TEST_VALUE(test_vec_5[0], "jello");
  EMP_TEST_VALUE(test_vec_5[1], "world");
  EMP_TEST_VALUE(test_vec_5[2], "!!");

  //Test nested arrays
  EM_ASM({emp_i.__outgoing_array = ([[4,5], [3,1], [7,8]])});
  emp::array<emp::array<int, 2>, 3> test_arr_6;
  emp::pass_array_to_cpp(test_arr_6);
  EMP_TEST_VALUE(test_arr_6[0][0], 4);
  EMP_TEST_VALUE(test_arr_6[0][1], 5);
  EMP_TEST_VALUE(test_arr_6[1][0], 3);
  EMP_TEST_VALUE(test_arr_6[1][1], 1);
  EMP_TEST_VALUE(test_arr_6[2][0], 7);
  EMP_TEST_VALUE(test_arr_6[2][1], 8);

  EM_ASM({emp_i.__outgoing_array = ([[4,5], [3,1], [7,8]])});
  emp::vector<emp::vector<int> > test_vec_6;
  emp::pass_vector_to_cpp(test_vec_6);
  EMP_TEST_VALUE(test_vec_6[0][0], 4);
  EMP_TEST_VALUE(test_vec_6[0][1], 5);
  EMP_TEST_VALUE(test_vec_6[1][0], 3);
  EMP_TEST_VALUE(test_vec_6[1][1], 1);
  EMP_TEST_VALUE(test_vec_6[2][0], 7);
  EMP_TEST_VALUE(test_vec_6[2][1], 8);

  //Test more deeply nested arrays
  EM_ASM({emp_i.__outgoing_array = ([[["Sooo", "many"], ["strings", "here"]],
				  [["and", "they're"], ["all", "nested"]],
				  [["in", "this"], ["nested", "array!"]]]);});
  emp::array<emp::array<emp::array<std::string, 2>, 2>, 3> test_arr_7;
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
