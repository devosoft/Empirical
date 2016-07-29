// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#include "../../emtools/init.h"

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/init.h"

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
  emp::Alert(w + x*y*z);
}

void TestFun2(double a, double b) {
  emp::Alert( a - b );
}

void TestFun3(std::string str1, int copies) {
  std::string out_str;
  for (int i = 0; i < copies; i++) out_str += str1;
  emp::Alert( out_str );
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
  emp::Alert( d.val2() );
  emp::Alert( d.word() );
  emp::Alert( d.val() );
  return d.val2();
}

//Test recursive JSON objects
void TestFun8(DoubleNestedJSDataObject d) {
  //Should be 8.8
  emp::Alert( d.obj2().obj2().val2() );
  //should be "a"
  emp::Alert( d.obj1().obj3().word() );
  //Should be 7
  emp::Alert( d.obj3().obj1().val() );
}


int main() {

  emp::Initialize();

  uint32_t fun_id1 = emp::JSWrap(TestFun1, "TestName1", false);
  uint32_t fun_id2 = emp::JSWrap(TestFun2, "TestName2", false);
  uint32_t fun_id3 = emp::JSWrap(TestFun3, "TestName3", false);
  uint32_t fun_id4 = emp::JSWrap(TestFun4, "TestName4", false);
  uint32_t fun_id5 = emp::JSWrap(TestFun5, "TestName5", false);
  uint32_t fun_id6 = emp::JSWrap(TestFun6, "TestName6", false);
  uint32_t fun_id7 = emp::JSWrap(TestFun7, "TestName7", false);
  uint32_t fun_id8 = emp::JSWrap(TestFun8, "TestName8", false);
  (void) fun_id4;
  (void) fun_id6;
  (void) fun_id7;
  (void) fun_id8;


  double in1 = 4.5;
  double in2 = 1.5;
  double val = EM_ASM_INT({ return emp.TestName4($0, $1); }, in1, in2);
  emp::Alert(in1, " / ", in2, " = ", val);

  EM_ASM_ARGS({
      alert( emp.Callback($0) );
    }, fun_id5);


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


}
