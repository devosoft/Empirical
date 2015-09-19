#include <emscripten.h>

#include "../../emtools/init.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/js_utils.h"
#include "../../tools/unit_tests.h"
#include "../../tools/command_line.h"
#include "../../tools/assert.h"

struct JSDataObject;

int main(int argc, char* argv[]) {
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Initialize();

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
  

  emp::PassArrayToJavascript(test_data);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[0];})
		 , "10");

  emp::PassArrayToJavascript(test_data_2);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[1].val;})
		 , "40");
  EMP_TEST_VALUE(
		 EM_ASM_DOUBLE_V({return emp.__incoming_array[1].val2;})
		 , "11.2");

  emp::PassArrayToJavascript(horrible_array);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[4][0][0];})
		 , "30");

  emp::PassArrayToJavascript(test_data_4);
  EMP_TEST_VALUE(
		 EM_ASM_INT_V({return emp.__incoming_array[1][0].val;})
		 , "40");
  EMP_TEST_VALUE(
		 EM_ASM_DOUBLE_V({return emp.__incoming_array[1][0].val2;})
		 , "11.2");
}
