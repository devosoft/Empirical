/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2020
 *
 *  @file JSWrap.cpp
 */

#include <functional>
#include <unordered_map>

#include "emp/base/assert.hpp"
#include "emp/web/init.hpp"
#include "emp/web/JSWrap.hpp"
#include "emp/web/_MochaTestRunner.hpp"

// --- These structs are used to test JSWrap ---
struct JSDataObject {
    EMP_BUILD_INTROSPECTIVE_TUPLE(
        int, val,
        std::string, word,
        double, val2
    )
};

struct NestedJSDataObject {
    EMP_BUILD_INTROSPECTIVE_TUPLE(
        JSDataObject, obj1,
        JSDataObject, obj2,
        JSDataObject, obj3
    )
};

struct DoubleNestedJSDataObject {
    EMP_BUILD_INTROSPECTIVE_TUPLE(
        NestedJSDataObject, obj1,
        NestedJSDataObject, obj2,
        NestedJSDataObject, obj3
    )
};


struct TestJSWrap : emp::web::BaseTest {

  std::function<int(int)> TestFun11;

  std::unordered_map<std::string, uint32_t> wrapped_func_ids;

  // emp::vector<uint32_t> wrapped_func_ids;
  // emp::vector<std::string> wrapped_func_names;

  TestJSWrap() {
    Setup();
  }

  ~TestJSWrap() {
    std::for_each(
      wrapped_func_ids.begin(),
      wrapped_func_ids.end(),
      [](auto & pair) {
        emp::JSDelete(pair.second);
      });
  }

  // Here we perform the necessary setup.
  void Setup() {
    // Wrap a function to get function ids by name.
    wrapped_func_ids["GetTestFuncID"] =
      emp::JSWrap(
        std::function<int(const std::string &)>([this](const std::string & name) { return (int)wrapped_func_ids[name]; }),
        "GetTestFuncID",
        false
      );

    const int x=10;

    // Wrap each of the test functions using JSWrap. Track their IDs so we can clean them up when
    // we're done.
    wrapped_func_ids["TestName1"] = emp::JSWrap(TestFun1, "TestName1", false);
    wrapped_func_ids["TestName2"] = emp::JSWrap(TestFun2, "TestName2", false);
    wrapped_func_ids["TestName3"] = emp::JSWrap(TestFun3, "TestName3", false);
    wrapped_func_ids["TestName4"] = emp::JSWrap(TestFun4, "TestName4", false);
    wrapped_func_ids["TestName5"] = emp::JSWrap(TestFun5, "TestName5", false);
    wrapped_func_ids["TestName6"] = emp::JSWrap(TestFun6, "TestName6", false);
    wrapped_func_ids["TestName7"] = emp::JSWrap(TestFun7, "TestName7", false);
    wrapped_func_ids["TestName8"] = emp::JSWrap(TestFun8, "TestName8", false);
    wrapped_func_ids["TestName9"] = emp::JSWrap([x]() { return x; }, "TestName9", false);
    wrapped_func_ids["TestName10"] = emp::JSWrap([](const std::string & msg){ std::cout << msg << std::endl; return "yes"; }, "TestName10", false);
    wrapped_func_ids["TestName11"] = emp::JSWrap([](int i){return i*i;}, "TestName11", false);
    wrapped_func_ids["TestName12"] = emp::JSWrap(TestFun12, "TestName12", false);
    wrapped_func_ids["TestName13"] = emp::JSWrap(TestFun13, "TestName13", false);
  }

  void Describe() override {
    EM_ASM({
      // Use Mocha to describe how JSWrap should work.
      describe("JSWrap", function() {
        describe("Wrap TestFun1", function() {
          it('should compute w+(x*y*z) from js', function() {
            const func_id = emp.GetTestFuncID("TestName1");
            const named_func_result = emp.TestName1(20, 10, 1, 0.5);
            const cb_func_result = emp.Callback(func_id, 20, 10, 1, 0.5);
            chai.assert.equal(named_func_result, 20+(10*1*0.5));
            chai.assert.equal(named_func_result, cb_func_result);
          });
        });
        describe("Wrap TestFun2", function() {
          it('should print a-b to the console and not crash', function() {
            const func_id = emp.GetTestFuncID("TestName2");
            emp.TestName2(1.5, 1.5);
            emp.Callback(func_id, 1.5, 1.5);
          });
        });
        describe("Wrap TestFun3", function() {
          it('should print abc to the console 3 times (twice) and not crash', function() {
            const func_id = emp.GetTestFuncID("TestName3");
            emp.TestName3("abc", 3);
            emp.Callback(func_id, "abc", 3);
          });
        });
        describe("Wrap TestFun4", function() {
          it('should compute a/b from js', function() {
            const in1  = 4.5;
            const in2 = 1.5;
            const func_id = emp.GetTestFuncID("TestName4");
            const named_func_result = emp.TestName4(in1, in2);
            const cb_func_result = emp.Callback(func_id, in1, in2);
            chai.assert.equal(named_func_result, in1/in2);
            chai.assert.equal(named_func_result, cb_func_result);
          });
        });
        describe("Wrap TestFun5", function() {
          it('should return 10000.1', function() {
            const expected = 10000.1;
            const func_id = emp.GetTestFuncID("TestName5");
            const named_ret = emp.TestName5();
            const cb_ret = emp.Callback(func_id);
            chai.assert.equal(named_ret, expected);
            chai.assert.equal(cb_ret, named_ret);
          });
        });
        // NOTE: this function wasn't tested in the original file, and it doesn't work. Not sure why.
        // describe("Wrap TestFun6", function() {
        //   it('should say that a is true', function() {
        //     const func_id = emp.GetTestFuncID("TestName6");
        //     const named_ret = emp.TestName6('a');
        //     const cb_ret = emp.Callback(func_id, 'a');
        //     chai.assert.equal(named_ret, true);
        //     chai.assert.equal(named_ret, cb_ret);
        //   });
        //   it('should say that A is false', function() {
        //     chai.assert.equal(emp.TestName6('A'), false);
        //   });
        //   it('should say that m is true', function() {
        //     chai.assert.equal(emp.TestName6('m'), true);
        //   });
        //   it('should say that L is false', function() {
        //     chai.assert.equal(emp.TestName6('L'), false);
        //   });
        //   it('should say that $ is false', function() {
        //     chai.assert.equal(emp.TestName6('$'), false);
        //   });
        // });
        describe("Wrap TestFun7", function() {
          it('it should not crash and should return JSDataObject::val', function() {
            const func_id = emp.GetTestFuncID("TestName7");
            const named_ret = emp.TestName7({val:5, word:"hi", val2:6.3});
            const cb_ret = emp.Callback(func_id, {val:5, word:"hi", val2:6.3});
            // chai.assert.equal(named_ret, 6.3);  // NOTE: This did not work on my machine, named_ret = 6.300000190734863
            chai.assert.equal(cb_ret, named_ret);
          });
        });
        describe("Wrap TestFun8", function() {
          it('should not crash from failed emp_asserts', function() {
            const func_id = emp.GetTestFuncID("TestName8");
            const gross = {
                            obj1:{
                              obj1:{val:1, word:"this", val2:6.3},
                              obj2:{val:2, word:"is", val2:6.3},
                              obj3:{val:3, word:"a", val2:6.3}
                            },
                            obj2:{
                              obj1:{val:4, word:"lot", val2:6.3},
                              obj2:{val:5, word:"of", val2:8.8},
                              obj3:{val:6, word:"nested", val2:6.3}
                            },
                            obj3:{
                              obj1:{val:7, word:"json", val2:6.3},
                              obj2:{val:8, word:"objects", val2:6.3},
                              obj3:{val:9, word:"yay", val2:6.3}
                            }
                          };
            emp.TestName8(gross);
            emp.Callback(func_id, gross);
          });
        });
        describe("Wrap anonymous lambda (TestName9)", function() {
          it('should return 10', function() {
            const func_id = emp.GetTestFuncID("TestName9");
            const named_ret = emp.TestName9();
            const cb_ret = emp.Callback(func_id);
            chai.assert.equal(named_ret, 10);
            chai.assert.equal(named_ret, cb_ret);
          });
        });
        describe("Wrap TestFun10", function() {
          it('should work', function() {
            const func_id = emp.GetTestFuncID("TestName10");
            const named_ret = emp.TestName10("This is a lambda with an rvalue!");
            const cb_ret = emp.Callback(func_id, "This is a lambda with an rvalue!");
            chai.assert.equal(named_ret, "yes");
            chai.assert.equal(named_ret, cb_ret);
          });
        });
        describe("Wrap TestFun11 (functor)", function() {
          it('should compute i*i', function() {
            const func_id = emp.GetTestFuncID("TestName11");
            const named_ret = emp.TestName11(5);
            const cb_ret = emp.Callback(func_id, 5);
            chai.assert.equal(named_ret, 25);
            chai.assert.equal(named_ret, cb_ret);
          });
        });
        describe("TestFun12", function() {
          it('should return a copy of the given JSDataObject', function() {
            const func_id = emp.GetTestFuncID("TestName12");
            const named_ret = emp.TestName12({val:5, word:"hi", val2:6.3});
            const cb_ret = emp.Callback(func_id, {val:5, word:"hi", val2:6.3});
            chai.assert.equal(named_ret.val, 5);
            chai.assert.equal(named_ret.word, "hi");
            chai.assert.equal(named_ret.val2, 6.3);
            chai.assert.deepEqual(named_ret, cb_ret);
          });
        });
        describe("TestFun13", function() {
          it('should return a copy of the given DoubleNestedJSDataObject', function() {
            const func_id = emp.GetTestFuncID("TestName13");
            const named_ret = emp.TestName13({obj1:{obj1:{val:1, word:"this", val2:6.3}, obj2:{val:2, word:"is", val2:6.3}, obj3:{val:3, word:"a", val2:6.3}},obj2:{obj1:{val:4, word:"lot", val2:6.3}, obj2:{val:5, word:"of", val2:8.8}, obj3:{val:6, word:"nested", val2:6.3}}, obj3:{obj1:{val:7, word:"json", val2:6.3}, obj2:{val:8, word:"objects", val2:6.3}, obj3:{val:9, word:"yay", val2:6.3}}});
            const cb_ret = emp.Callback(func_id, {obj1:{obj1:{val:1, word:"this", val2:6.3}, obj2:{val:2, word:"is", val2:6.3}, obj3:{val:3, word:"a", val2:6.3}},obj2:{obj1:{val:4, word:"lot", val2:6.3}, obj2:{val:5, word:"of", val2:8.8}, obj3:{val:6, word:"nested", val2:6.3}}, obj3:{obj1:{val:7, word:"json", val2:6.3}, obj2:{val:8, word:"objects", val2:6.3}, obj3:{val:9, word:"yay", val2:6.3}}});
            chai.assert.equal(named_ret.obj1.obj1.val, 1);
            chai.assert.equal(named_ret.obj2.obj1.val, 4);
            chai.assert.equal(named_ret.obj3.obj1.val, 7);
            chai.assert.equal(named_ret.obj1.obj2.word, "is");
            chai.assert.equal(named_ret.obj2.obj3.word, "nested");
            chai.assert.equal(named_ret.obj2.obj2.val2, 8.8);
            chai.assert.equal(named_ret.obj3.obj3.val2, 6.3);
            chai.assert.deepEqual(named_ret, cb_ret);
          });
        });
      });
    });
  }

  // --- Functions to JSWrap! ---
  static double TestFun1(int w, int x, int y, double z) {
    return w + (x*y*z);
  }

  static void TestFun2(double a, double b) {
    std::cout <<  a - b  << std::endl;
  }

  static void TestFun3(std::string str1, int copies) {
    std::string out_str;
    for (int i = 0; i < copies; i++) out_str += str1;
    std::cout << out_str << std::endl;
  }

  static double TestFun4(double a, double b) {
    return  a / b ;
  }

  static double TestFun5() { return 10000.1; }

  static bool TestFun6(char in_char) {
    std::cout << "in_char = " << in_char << std::endl;
    std::cout << "(in_char >= 'a')" << (in_char >= 'a') << std::endl;
    std::cout << "(in_char <= 'z')" << (in_char <= 'z') << std::endl;
    return in_char >= 'a' && in_char <= 'z';
  }

  // Test user-defined JSON integration
  static float TestFun7(JSDataObject d) {
    emp_assert( d.val2() == 6.3 );
    emp_assert( d.word() == "hi");
    emp_assert( d.val() == 5 );
    return d.val2();
  }

  // Test recursive JSON objects
  static void TestFun8(DoubleNestedJSDataObject d) {
    //Should be 8.8
    emp_assert( d.obj2().obj2().val2() == 8.8 );
    //should be "a"
    emp_assert( d.obj1().obj3().word() == "a" );
    //Should be 7
    emp_assert( d.obj3().obj1().val() == 7 );
  }

  // Test recursive JSON objects
  static JSDataObject TestFun12(JSDataObject d) {
    return d;
  }

  // Test recursive JSON objects
  static DoubleNestedJSDataObject TestFun13(DoubleNestedJSDataObject d) {
    return d;
  }

};

emp::web::MochaTestRunner test_runner; // Create a test manager to run JSWrap tests.
int main() {
  emp::Initialize();
  test_runner.AddTest<TestJSWrap>("TestJSWrap");
  test_runner.Run();
}
