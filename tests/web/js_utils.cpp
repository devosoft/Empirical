/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file js_utils.cpp
 */

#include <array>
#include <cassert>
#include <string>

#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"
#include "emp/testing/unit_tests.hpp"
#include "emp/web/init.hpp"
#include "emp/web/js_utils.hpp"
#include "emp/web/JSWrap.hpp"
#include "emp/web/_MochaTestRunner.hpp"

// This file tests source/web/js_utils.h (using Mocha + Karma framework)
// - pass_array_to_javascript
// - pass_array_to_cpp
// - pass_map_to_javascript

// Test pass_array_to_javascript functions
struct TestPassArrayToJavaScript : public emp::web::BaseTest {

  struct JSDataObject {
    EMP_BUILD_INTROSPECTIVE_TUPLE(
          int, val,
          std::string, word,
          double, val2
      )
  };

  JSDataObject test_obj_1{};
  JSDataObject test_obj_2{};

  emp::vector<uint32_t> wrapped_fun_ids;

  TestPassArrayToJavaScript() { Setup(); }

  ~TestPassArrayToJavaScript() {
    // cleanup wrapped functions
    std::for_each(
      wrapped_fun_ids.begin(),
      wrapped_fun_ids.end(),
      [](uint32_t fun_id) {
        emp::JSDelete(fun_id);
      }
    );
  }

  void Setup() {
    test_obj_1.val() = 10;
    test_obj_1.word() = "hi";
    test_obj_1.val2() = 4.4;

    test_obj_2.val() = 40;
    test_obj_2.word() = "hi2";
    test_obj_2.val2() = 11.2;

    // Configure functions that can be called from JS that pass various C++ vectors/arrays to JS
    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::vector<emp::vector<emp::vector<double> > > nested_vec{{{1,2,3},{4,5,6}}};
          emp::pass_array_to_javascript(nested_vec);
        },
        "PassNestedVectorToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::vector<int> int_vec{5,1,2,3,6};
          emp::pass_array_to_javascript(int_vec);
        },
        "PassIntVectorToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::vector<std::string> string_vec{"a", "vector", "of", "strings"};
          emp::pass_array_to_javascript(string_vec);
        },
        "PassStringVectorToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::array<int32_t, 3> test_data{{10,30,60}};
          emp::pass_array_to_javascript(test_data);
        },
        "PassArrayIntToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [this]() {
          emp::array<JSDataObject, 2> test_data_2{{test_obj_1, test_obj_2}};
          emp::pass_array_to_javascript(test_data_2);
        },
        "PassArrayJSDataObjectToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::array<emp::array<std::string, 5>, 1> string_arr{{{{"do", "strings", "work", "in", "arrays?"}}}};
          emp::pass_array_to_javascript(string_arr);
        },
        "PassNestedArrayStringToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::array<emp::array<emp::array<int, 2>, 1>, 5> horrible_array{{{{{{0,0}}}}, {{{{0,10}}}}, {{{{10,10}}}}, {{{{20,20}}}}, {{{{30, 30}}}}}};
          emp::pass_array_to_javascript(horrible_array);
        },
        "PassNestedArrayIntToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [this]() {
          emp::array<emp::array<JSDataObject, 2>, 2> test_data_4{{{{test_obj_1, test_obj_2}}, {{test_obj_2, test_obj_2}}}};
          emp::pass_array_to_javascript(test_data_4);
        },
        "PassNestedArrayJSDataObjectToJS",
        false
      )
    );
  }

  void Describe() override {

    // Test passing arrays to javascript
    EM_ASM({
      describe("pass_array_to_javascript", function() {
        it("should pass nested C++ vectors to javascript", function() {
          emp.PassNestedVectorToJS(); // {{1,2,3},{4,5,6}}}
          const incoming = emp_i.__incoming_array;
          chai.assert.equal(incoming[0][0][0], 1);
          chai.assert.equal(incoming[0][0][1], 2);
          chai.assert.equal(incoming[0][0][2], 3);
          chai.assert.equal(incoming[0][1][0], 4);
          chai.assert.equal(incoming[0][1][1], 5);
          chai.assert.equal(incoming[0][1][2], 6);
        });
        it("should pass emp::vector<int> to javascript", function() {
          emp.PassIntVectorToJS(); // {5,1,2,3,6}
          const incoming = emp_i.__incoming_array;
          chai.assert.deepEqual(incoming, [5, 1, 2, 3, 6]);
        });
        it("should pass emp::vector<std::string> to javascript", function() {
          emp.PassStringVectorToJS(); // {"a", "vector", "of", "strings"}
          const incoming = emp_i.__incoming_array;
          chai.assert.deepEqual(incoming, ['a', 'vector', 'of', 'strings']);
        });
        it("should pass emp::array<int, N> to javascript", function() {
          emp.PassArrayIntToJS(); // {{10,30,60}}
          const incoming = emp_i.__incoming_array;
          chai.assert.deepEqual(incoming, [10, 30, 60]);
        });
        it("should pass emp::array<JSDataObject, N> to javascript", function() {
          emp.PassArrayJSDataObjectToJS();
          const incoming = emp_i.__incoming_array;
          chai.assert.deepEqual(incoming, [{val: 10, word: 'hi', val2: 4.4}, {val: 40, word: 'hi2', val2: 11.2}]);
        });
        it("should pass emp::array<emp::array<std::string, N0>, N1> to javascript", function() {
          emp.PassNestedArrayStringToJS();
          const incoming = emp_i.__incoming_array;
          // chai.assert.deepEqual(incoming, ['do', 'strings', 'work', 'in', 'arrays?']);
          chai.assert.equal(incoming[0][3], "in")
        });
        it("should pass emp::array<emp::array<emp::array<int, N0>, N1>, N2> to javascript", function() {
          emp.PassNestedArrayIntToJS(); // {{{{{{0,0}}}}, {{{{0,10}}}}, {{{{10,10}}}}, {{{{20,20}}}}, {{{{30, 30}}}}}}
          const incoming = emp_i.__incoming_array;
          chai.assert.equal(incoming[4][0][0], 30);
        });
        it("should pass emp::array<emp::array<JSDataObject, N0>, N1> to javascript", function() {
          emp.PassNestedArrayJSDataObjectToJS(); // {{{{test_obj_1, test_obj_2}}, {{test_obj_2, test_obj_2}}}};
          const incoming = emp_i.__incoming_array;
          chai.assert.equal(incoming[1][0].val, 40);
          chai.assert.equal(incoming[1][0].val2, 11.2);
        });
      });
    });
  }

};


// Test pass_array_to_cpp (from javascript) functions
// Because we want to check values on the C++ end, this test relies on raw C++ asserts (from cassert)
// to check values.
struct TestPassArrayToCpp : public emp::web::BaseTest {

  TestPassArrayToCpp() { Setup(); }

  void Setup() {

    // Test ints
    EM_ASM({emp_i.__outgoing_array = ([5, 1, 3])});
    emp::array<int, 3> test_arr_1;
    emp::pass_array_to_cpp(test_arr_1);

    #ifdef __EMSCRIPTEN__
    assert(test_arr_1[0] == 5);
    assert(test_arr_1[1] == 1);
    assert(test_arr_1[2] == 3);
    #endif

    // Test floats
    EM_ASM({emp_i.__outgoing_array = ([5.2, 1.5, 3.1])});
    emp::array<float, 3> test_arr_2;
    emp::pass_array_to_cpp(test_arr_2);

    #ifdef __EMSCRIPTEN__
    assert(emp::to_string(test_arr_2[0]) == emp::to_string(5.2));
    assert(test_arr_2[1] == 1.5);
    assert(emp::to_string(test_arr_2[2]) == emp::to_string(3.1));
    #endif

    // Test doubles
    EM_ASM({emp_i.__outgoing_array = ([5.2, 1.5, 3.1])});
    emp::array<double, 3> test_arr_3;
    emp::pass_array_to_cpp(test_arr_3);

    #ifdef __EMSCRIPTEN__
    assert(test_arr_3[0] == 5.2);
    assert(test_arr_3[1] == 1.5);
    assert(test_arr_3[2] == 3.1);
    #endif

    // Test doubles in vector
    EM_ASM({emp_i.__outgoing_array = ([5.3, 1.6, 3.2])});
    emp::vector<double> test_vec;
    emp::pass_vector_to_cpp(test_vec);

    #ifdef __EMSCRIPTEN__
    assert(test_vec[0] == 5.3);
    assert(test_vec[1] == 1.6);
    assert(test_vec[2] == 3.2);
    #endif

    // Test chars
    EM_ASM({emp_i.__outgoing_array = (["h", "i", "!"])});
    emp::array<char, 3> test_arr_4;
    emp::pass_array_to_cpp(test_arr_4);
    #ifdef __EMSCRIPTEN__
    assert(test_arr_4[0] == 'h');
    assert(test_arr_4[1] == 'i');
    assert(test_arr_4[2] == '!');
    #endif

    emp::vector<char> test_vec_4;
    emp::pass_vector_to_cpp(test_vec_4);
    #ifdef __EMSCRIPTEN__
    assert(test_vec_4[0] == 'h');
    assert(test_vec_4[1] == 'i');
    assert(test_vec_4[2] == '!');
    #endif

    // Test std::strings
    EM_ASM({emp_i.__outgoing_array = (["jello", "world", "!!"])});
    emp::array<std::string, 3> test_arr_5;
    emp::pass_array_to_cpp(test_arr_5);
    #ifdef __EMSCRIPTEN__
    assert(test_arr_5[0] == "jello");
    assert(test_arr_5[1] == "world");
    assert(test_arr_5[2] == "!!");
    #endif

    emp::vector<std::string> test_vec_5;
    emp::pass_vector_to_cpp(test_vec_5);
    #ifdef __EMSCRIPTEN__
    assert(test_vec_5[0] == "jello");
    assert(test_vec_5[1] == "world");
    assert(test_vec_5[2] == "!!");
    #endif

    // Test nested arrays
    EM_ASM({emp_i.__outgoing_array = ([[4,5], [3,1], [7,8]])});
    emp::array<emp::array<int, 2>, 3> test_arr_6;
    emp::pass_array_to_cpp(test_arr_6);
    #ifdef __EMSCRIPTEN__
    assert(test_arr_6[0][0] == 4);
    assert(test_arr_6[0][1] == 5);
    assert(test_arr_6[1][0] == 3);
    assert(test_arr_6[1][1] == 1);
    assert(test_arr_6[2][0] == 7);
    assert(test_arr_6[2][1] == 8);
    #endif

    // Test nested vectors
    EM_ASM({emp_i.__outgoing_array = ([[4,5], [3,1], [7,8]])});
    emp::vector<emp::vector<int> > test_vec_6;
    emp::pass_vector_to_cpp(test_vec_6);
    #ifdef __EMSCRIPTEN__
    assert(test_vec_6[0][0] == 4);
    assert(test_vec_6[0][1] == 5);
    assert(test_vec_6[1][0] == 3);
    assert(test_vec_6[1][1] == 1);
    assert(test_vec_6[2][0] == 7);
    assert(test_vec_6[2][1] == 8);
    #endif

    // Test more deeply nested arrays
    EM_ASM({emp_i.__outgoing_array = ([[["Sooo", "many"], ["strings", "here"]],
      [["and", "they're"], ["all", "nested"]],
      [["in", "this"], ["nested", "array!"]]]);});
    emp::array<emp::array<emp::array<std::string, 2>, 2>, 3> test_arr_7;
    emp::pass_array_to_cpp(test_arr_7);
    #ifdef __EMSCRIPTEN__
    assert(test_arr_7[0][0][0] == "Sooo");
    assert(test_arr_7[0][0][1] == "many");
    assert(test_arr_7[0][1][0] == "strings");
    assert(test_arr_7[0][1][1] == "here");
    assert(test_arr_7[1][0][0] == "and");
    assert(test_arr_7[1][0][1] == "they're");
    assert(test_arr_7[1][1][0] == "all");
    assert(test_arr_7[1][1][1] == "nested");
    assert(test_arr_7[2][0][0] == "in");
    assert(test_arr_7[2][0][1] == "this");
    assert(test_arr_7[2][1][0] == "nested");
    assert(test_arr_7[2][1][1] == "array!");
    #endif
  }

};


// Test pass_map_to_javascript functions
struct TestPassMapToJavascript : public emp::web::BaseTest {

  emp::vector<uint32_t> wrapped_fun_ids;

  TestPassMapToJavascript() { Setup(); }

  ~TestPassMapToJavascript() {
    // cleanup wrapped functions
    std::for_each(
      wrapped_fun_ids.begin(),
      wrapped_fun_ids.end(),
      [](uint32_t fun_id) {
        emp::JSDelete(fun_id);
      }
    );
  }

  void Setup() {
    // Configure functions that can be called from JS that pass various C++ maps/pairs of arrays to JS
    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::map<std::string, std::string> testMap = {{"test1", "a"}, {"test2", "b"}, {"test3", "c"}, {"test4", "d"}, {"test5", "e"}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassStringStringMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::map<std::string, double> testMap = {{"test1", 1.01}, {"test2", 2.02}, {"test3", 3.03}, {"test4", 4.04}, {"test5", 5.05}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassStringDoubleMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::map<std::string, int> testMap = {{"test1", 1}, {"test2", 2}, {"test3", 3}, {"test4", 4}, {"test5", 5}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassStringIntMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::map<double, std::string> testMap = {{1.01, "test1"}, {2.02, "test2"}, {3.03, "test3"}, {4.04, "test4"}, {5.05, "test5"}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassDoubleStringMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::map<int, std::string> testMap = {{1, "test1"}, {2, "test2"}, {3, "test3"}, {4, "test4"}, {5, "test5"}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassIntStringMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        []() {
          emp::vector<double> arr1 = {1.01, 2.02, 3.03};
          emp::vector<double> arr2 = {10.01, 20.02, 30.03};
          emp::vector<double> arr3 = {100.01, 200.02, 300.03};
          emp::map<std::string, emp::vector<double>> testMap = {{"test1", arr1}, {"test2", arr2}, {"test3", arr3}};
          emp::pass_map_to_javascript(testMap);
        },
        "PassStringVectorMapToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::array<std::string, 5> keysCpp = {"test1", "test2", "test3", "test4", "test5"};
          emp::array<double, 5> vals = {10.1, 20.2, 30.3, 40.4, 50.5};
          emp::pass_map_to_javascript(keysCpp, vals);
        },
        "PassStringDoubleArraysToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::array<std::string, 5> keysCpp = {"test1", "test2", "test3", "test4", "test5"};
          emp::array<std::string, 5> vals = {"red", "blue", "blue", "yellow", "purple"};
          emp::pass_map_to_javascript(keysCpp, vals);
        },
        "PassTwoStringArraysToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::array<double, 5> keysCpp = {1.01, 2.02, 3.03, 4.04, 5.05};
          emp::array<double, 5> vals = {100.5, 200.5, 300.5, 400.5, 500.5};
          emp::pass_map_to_javascript(keysCpp, vals);
        },
        "PassTwoDoubleArraysToJS",
        false
      )
    );

    wrapped_fun_ids.emplace_back(
      emp::JSWrap(
        [](){
          emp::array<int, 5> keysCpp = {1, 2, 3, 4, 5};
          emp::array<int, 5> vals = {10, 20, 30, 40, 50};
          emp::pass_map_to_javascript(keysCpp, vals);
        },
        "PassTwoIntArraysToJS",
        false
      )
    );
  }

  void Describe() override {

    // Test passing maps to javascript
    EM_ASM({
      describe("pass_map_to_javascript", function() {
        it("should pass emp::map<std::string, std::string> to javascript", function() {
          emp.PassStringStringMapToJS(); // {{"test1", "a"}, {"test2", "b"}, {"test3", "c"}, {"test4", "d"}, {"test5", "e"}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": "a", "test2": "b", "test3": "c", "test4": "d", "test5": "e"} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::map<std::string, double> to javascript", function() {
          emp.PassStringDoubleMapToJS(); // {{"test1", 1.01}, {"test2", 2.02}, {"test3", 3.03}, {"test4", 4.04}, {"test5", 5.05}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": 1.01, "test2": 2.02, "test3": 3.03, "test4": 4.04, "test5": 5.05} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::map<std::string, int> to javascript", function() {
          emp.PassStringIntMapToJS(); // {{"test1", 1}, {"test2", 2}, {"test3", 3}, {"test4", 4}, {"test5", 5}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": 1, "test2": 2, "test3": 3, "test4": 4, "test5": 5} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::map<double, std::string> to javascript", function() {
          emp.PassDoubleStringMapToJS(); // {{1.01, "test1"}, {2.02, "test2"}, {3.03, "test3"}, {4.04, "test4"}, {5.05, "test5"}};
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {1.01: "test1", 2.02: "test2", 3.03: "test3", 4.04: "test4", 5.05: "test5"} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::map<int, std::string> to javascript", function() {
          emp.PassIntStringMapToJS(); // {{1, "test1"}, {2, "test2"}, {3, "test3"}, {4, "test4"}, {5, "test5"}};
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {1: "test1", 2: "test2", 3: "test3", 4: "test4", 5: "test5"} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::map<std::string, emp::vector<double>> to javascript", function() {
          emp.PassStringVectorMapToJS(); // {{"test1", {1.01, 2.02, 3.03}},  {"test2", {10.01, 20.02, 30.03}}, {"test3", {100.01, 200.02, 300.03}}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": [1.01, 2.02, 3.03], "test2": [10.01, 20.02, 30.03], "test3": [100.01, 200.02, 300.03]} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass emp::array<std::string, 5> and emp::array<double, 5> to javascript as a map", function() {
          emp.PassStringDoubleArraysToJS(); // {{"test1", 10.1}, {"test2", 20.2}, {"test3", 30.3}, {"test4", 40.4}, {"test5", 50.5}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": 10.1, "test2": 20.2, "test3": 30.3, "test4": 40.4, "test5": 50.5} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass two emp::array<std::string, 5> to javascript as a map", function() {
          emp.PassTwoStringArraysToJS(); // {{"test1", "red"}, {"test2", "blue"}, {"test3", "blue"}, {"test4", "yellow"}, {"test5", "purple"}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {"test1": "red", "test2": "blue", "test3": "blue", "test4": "yellow", "test5": "purple"} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass two emp::array<double, 5> to javascript as a map", function() {
          emp.PassTwoDoubleArraysToJS(); // {{1.01, 100.5}, {2.02, 200.5}, {3.03, 300.5}, {4.04, 400.5}, {5.05, 500.5}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {1.01: 100.5, 2.02: 200.5, 3.03: 300.5, 4.04: 400.5, 5.05: 500.5} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
        it("should pass two emp::array<int, 5> to javascript as a map", function() {
          emp.PassTwoIntArraysToJS(); // {{1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}}
          const incoming = emp_i.__incoming_map;
          const jsVersion = ( {1: 10, 2: 20, 3: 30, 4: 40, 5: 50} );
          chai.assert.deepEqual(incoming, jsVersion);
        });
      });
    });
  }
};

emp::web::MochaTestRunner test_runner;

int main() {
  emp::Initialize();

  test_runner.AddTest<TestPassArrayToJavaScript>("pass_array_to_javascript");
  test_runner.AddTest<TestPassArrayToCpp>("pass_array_to_cpp");
  test_runner.AddTest<TestPassMapToJavascript>("pass_map_to_javascript");

  test_runner.Run();
}
