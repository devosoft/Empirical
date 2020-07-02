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
#include "d3_testing_utils.h"

struct Test_Transition : BaseTest {

  void Setup() { ; }

  void Describe() {
    EM_ASM({
      describe("transition", function() {
        it("should work", function() {
          chai.assert(false);
        });
      });
    });
  }
};

emp::web::Document doc("test_d3_init");
TestManager manager;

int main() {
  D3::internal::get_emp_d3();
  manager.AddTest<Test_Transition>();

  manager.OnBeforeEachTest([]() { ResetD3Context(); });
  manager.Run();

  return 0;
}
