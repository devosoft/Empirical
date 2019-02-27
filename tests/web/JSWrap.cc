//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include <functional>

#include "../../tests2/unit_tests.h"
#include "base/assert.h"
#include "web/init.h"
#include "web/JSWrap.h"

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


void TestFun1(int w, int x, int y, double z) {
  std::cout << w + x*y*z << std::endl;
}

void TestFun2(double a, double b) {
  std::cout <<  a - b  << std::endl;
}

void TestFun3(std::string str1, int copies) {
  std::string out_str;
  for (int i = 0; i < copies; i++) out_str += str1;
  std::cout << out_str << std::endl;
}

double TestFun4(double a, double b) {
  return  a / b ;
}

double TestFun5() { return 10000.1; }

bool TestFun6(char in_char) {
  return in_char >= 'a' && in_char <= 'z';
}

//Test user-defined JSON integration
float TestFun7(JSDataObject d) {
  emp_assert( d.val2() == 6.3 );
  emp_assert( d.word() == "hi");
  emp_assert( d.val() == 5 );
  return d.val2();
}

//Test recursive JSON objects
void TestFun8(DoubleNestedJSDataObject d) {
  //Should be 8.8
  emp_assert( d.obj2().obj2().val2() == 8.8 );
  //should be "a"
  emp_assert( d.obj1().obj3().word() == "a" );
  //Should be 7
  emp_assert( d.obj3().obj1().val() == 7 );
}

//Test recursive JSON objects
JSDataObject TestFun12(JSDataObject d) {
  return d;
}

//Test recursive JSON objects
DoubleNestedJSDataObject TestFun13(DoubleNestedJSDataObject d) {
  return d;
}

int main() {

  emp::Initialize();
  [[maybe_unused]] bool verbose = true;
  int x = 10;
  auto lambda = [x](){EMP_TEST_VALUE(x, 10);};

  std::function<int(int)> TestFun11 = [](int i){return i*i;};

  uint32_t fun_id1 = emp::JSWrap(TestFun1, "TestName1", false);
  uint32_t fun_id2 = emp::JSWrap(TestFun2, "TestName2", false);
  uint32_t fun_id3 = emp::JSWrap(TestFun3, "TestName3", false);
  uint32_t fun_id4 = emp::JSWrap(TestFun4, "TestName4", false);
  uint32_t fun_id5 = emp::JSWrap(TestFun5, "TestName5", false);
  uint32_t fun_id6 = emp::JSWrap(TestFun6, "TestName6", false);
  uint32_t fun_id7 = emp::JSWrap(TestFun7, "TestName7", false);
  uint32_t fun_id8 = emp::JSWrap(TestFun8, "TestName8", false);
  uint32_t fun_id9 = emp::JSWrap(lambda, "TestName9", false);
  uint32_t fun_id10 = emp::JSWrap([](std::string msg){std::cout << msg << std::endl; return "yes";}, "TestName10", false);
  uint32_t fun_id11 = emp::JSWrap(TestFun11, "TestName11", false);
  uint32_t fun_id12 = emp::JSWrap(TestFun12, "TestName12", false);
  uint32_t fun_id13 = emp::JSWrap(TestFun13, "TestName13", false);
  (void) fun_id1;
  (void) fun_id2;
  (void) fun_id3;
  (void) fun_id4;
  (void) fun_id5;
  (void) fun_id6;
  (void) fun_id7;
  (void) fun_id8;
  (void) fun_id9;
  (void) fun_id10;
  (void) fun_id11;
  (void) fun_id12;
  (void) fun_id13;

  double in1 = 4.5;
  double in2 = 1.5;
  double val = EM_ASM_INT({ return emp.TestName4($0, $1); }, in1, in2);
  emp_assert(in1/in2 == val);

  EMP_TEST_VALUE(EM_ASM_DOUBLE({
      return emp.Callback($0);
  }, fun_id5), 10000.1);


  EM_ASM_ARGS({
      emp.Callback($0, 5, 17, 4, 1.5);
    }, fun_id1);

  EM_ASM_ARGS({
      emp.Callback($0, "abc", 3);
    }, fun_id3);

  EM_ASM_ARGS({
      emp.Callback($0, 4.0, 2.5);
    }, fun_id2);

  EM_ASM({
      emp.TestName1(20, 10, 1, 0.5);
    });

  EM_ASM({
      emp.TestName2(1.5, 1.5);
    });

  EM_ASM({
      emp.TestName7({val:5, word:"hi", val2:6.3});
    });

  EM_ASM({
      emp.TestName8( {obj1:{obj1:{val:1, word:"this", val2:6.3}, obj2:{val:2, word:"is", val2:6.3}, obj3:{val:3, word:"a", val2:6.3}},obj2:{obj1:{val:4, word:"lot", val2:6.3}, obj2:{val:5, word:"of", val2:8.8}, obj3:{val:6, word:"nested", val2:6.3}}, obj3:{obj1:{val:7, word:"json", val2:6.3}, obj2:{val:8, word:"objects", val2:6.3}, obj3:{val:9, word:"yay", val2:6.3}}});
  });

  EM_ASM({
    emp.TestName9();
  });

  EM_ASM({
    console.assert(emp.TestName10("This is a lambda with an rvalue!") == "yes");
  });

  EMP_TEST_VALUE(EM_ASM_INT_V({
    return emp.TestName11(5);
  }), 25);

  EM_ASM({
    ret = emp.TestName12({val:5, word:"hi", val2:6.3});
    console.assert(ret.val == 5, ret);
    console.assert(ret.word == "hi", ret);
    console.assert(ret.val2 == 6.3, ret);
  });

  EM_ASM({
    ret = emp.TestName13( {obj1:{obj1:{val:1, word:"this", val2:6.3}, obj2:{val:2, word:"is", val2:6.3}, obj3:{val:3, word:"a", val2:6.3}},obj2:{obj1:{val:4, word:"lot", val2:6.3}, obj2:{val:5, word:"of", val2:8.8}, obj3:{val:6, word:"nested", val2:6.3}}, obj3:{obj1:{val:7, word:"json", val2:6.3}, obj2:{val:8, word:"objects", val2:6.3}, obj3:{val:9, word:"yay", val2:6.3}}});
    console.assert(ret.obj1.obj1.val == 1, ret);
    console.assert(ret.obj2.obj1.val == 4, ret);
    console.assert(ret.obj3.obj1.val == 7, ret);
    console.assert(ret.obj1.obj2.word == "is", ret);
    console.assert(ret.obj2.obj3.word == "nested", ret);
    console.assert(ret.obj2.obj2.val2 == 8.8, ret);
    console.assert(ret.obj3.obj3.val2 == 6.3, ret);

  });


}
