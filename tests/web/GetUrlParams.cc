//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2019.
//  Released under the MIT Software license; see doc/LICENSE
#include <functional>

#include "../../tests2/unit_tests.h"
#include "base/assert.h"
#include "web/init.h"
#include "web/JSWrap.h"

#include "base/assert.h"
#include "web/UrlParams.h"

int main() {

  emp::Initialize();

  EM_ASM({
    global.location = Object();
    global.location.search = "test1=result&test2=result&test3=123&test1=nope";
  });

  const auto params = emp::web::GetUrlParams();
  emp_assert(params.find("test1")->second == "result");
  emp_assert(params.find("test2")->second == "result");
  emp_assert(params.find("test3")->second == "123");
  emp_assert(params.find("nope") == params.end());
  std::cout << "Success!" << std::endl;
  
}
