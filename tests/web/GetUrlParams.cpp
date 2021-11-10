/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2019
 *
 *  @file GetUrlParams.cpp
 */

#include <functional>

#include "emp/base/assert.hpp"
#include "emp/base/assert.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/web/init.hpp"
#include "emp/web/JSWrap.hpp"
#include "emp/web/NodeDomShim.hpp"
#include "emp/web/UrlParams.hpp"

const emp::web::NodeDomShim shim;

int main() {

  emp::Initialize();

  EM_ASM({
    global.location = Object();
    global.location.search = (
      "?test1=val1" +
      "&test4" +
      "&test5=" +
      "&test2=val1+" +
      "&test3=1+23" +
      "&=bad" +
      "&=" +
      "&+=" +
      "&+%20=" +
      "&=+" +
      "&+=+" +
      "&test1=val2+val3" +
      "&_positional=p1++p2" +
      "&_positional=p3+" +
      "&bad+bad=illegal" +
      "&bad+%20bad=illegal" +
      "&+bad=illegal" +
      "&bad+=illegal" +
      "&string1=no%20break" +
      "&string2=breaks%20here+and+there" +
      "&string3=sneakyspace%20+more"
    );
  });

  emp::ArgManager am(emp::web::GetUrlParams());

  am.PrintDiagnostic();

 // TODO: rewrite this to use catch/REQUIRE
 // (since this breaks without rhyme or reason and just crashes to indicate
 //  a failed test rather than failing gracefully for some checks)
  emp_assert(*am.UseArg("test1") == emp::vector<std::string>({"val1"}));
  emp_assert(*am.UseArg("test1") == emp::vector<std::string>({"val2","val3"}));
  emp_assert(!am.UseArg("test1"));

  emp_assert(*am.UseArg("test2") == emp::vector<std::string>({"val1", ""}));
  emp_assert(!am.UseArg("test2"));

  emp_assert(*am.UseArg("test3") == emp::vector<std::string>({"1","23"}));
  emp_assert(!am.UseArg("test3"));

  emp_assert(*am.UseArg("test4") == emp::vector<std::string>());
  emp_assert(!am.UseArg("test4"));

  emp_assert(*am.UseArg("test5") == emp::vector<std::string>({""}));
  emp_assert(!am.UseArg("test5"));

  emp_assert(
    *am.UseArg("_positional") == emp::vector<std::string>({"p1","","p2","p3",""})
  );
  emp_assert(!am.UseArg("_positional"));

  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty=bad"})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty="})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty="})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty="})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty="})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"_empty="})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"bad bad=illegal"})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"bad  bad=illegal"})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({" bad=illegal"})
  );
  emp_assert(
    *am.UseArg("_illegal") == emp::vector<std::string>({"bad =illegal"})
  );
  emp_assert(*am.UseArg("string1") == emp::vector<std::string>({"no break"}));
  emp_assert(!am.UseArg("string1"));

  std::cout << __FILE__ << ":" << __LINE__ << std::endl;
  emp_assert(*am.UseArg("string2") == emp::vector<std::string>({"breaks here", "and", "there"}));

  emp_assert(!am.UseArg("string2"));

  emp_assert(*am.UseArg("string3") == emp::vector<std::string>({"sneakyspace ", "more"}));

  emp_assert(!am.UseArg("string3"));

}
