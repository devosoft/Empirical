#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/JSWrap.h"
#include "../../emtools/init.h"

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

//Test JSDataObject integration
float TestFun7(JSDataObject d) {
  emp::Alert( d.val2() );
  emp::Alert( d.word() );
  emp::Alert( d.val() );
  return d.val2();
}

int main() {

  emp::Initialize();

  uint32_t fun_id1 = emp::JSWrap(TestFun1, "TestName1", false);
  uint32_t fun_id2 = emp::JSWrap(TestFun2, "TestName2", false);
  uint32_t fun_id3 = emp::JSWrap(TestFun3, "TestName3", false);
  uint32_t fun_id4 = emp::JSWrap(TestFun4, "TestName4", false);
  uint32_t fun_id5 = emp::JSWrap(TestFun5, "TestName5", false);
  uint32_t fun_id6 = emp::JSWrap(TestFun6, "TestName6", false);
  // uint32_t fun_id7 = emp::JSWrap(TestFun7, "TestName7", false);
  (void) fun_id4;
  (void) fun_id6;


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
  
}
