#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/keyname_utils.hpp"

#include <sstream>
#include <iostream>
#include <string>


TEST_CASE("Test keyname_utils", "[tools]")
{

  // test unpack

  emp::keyname::unpack_t goal{
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
  };

  std::string name;

  name = "seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  // reorderings
  name = "foobar=20+seed=100+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "_hash=asdf+foobar=20+seed=100+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  // should ignore path
  name = "path/seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "~/more=path/+blah/seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "just/a/regular/file.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file.pdf", ""},
    {"_", "just/a/regular/file.pdf"}
  }));

  name = "key/with/no+=value/file+ext=.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file", ""},
    {"ext", ".pdf"},
    {"_", "key/with/no+=value/file+ext=.pdf"}
  }));

  name = "multiple/=s/file=biz=blah+ext=.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file", "biz=blah"},
    {"ext", ".pdf"},
    {"_", "multiple/=s/file=biz=blah+ext=.pdf"}
  }));

  // test pack
  // reorderings
  REQUIRE( (emp::keyname::pack({
     {"seed", "100"},
     {"foobar", "20"},
     {"_hash", "asdf"},
     {"ext", ".txt"}
    })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
   );

  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"seed", "100"},
      {"foobar", "20"},
      {"ext", ".txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"foobar", "20"},
      {"ext", ".txt"},
      {"seed", "100"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  // different values
  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "foobar=blip+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "a100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "foobar=blip+seed=a100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"aseed", "a100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "aseed=a100+foobar=blip+_hash=asdf+ext=.txt"
  );

  // should ignore "_" key
  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "foobar=20+seed=100+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "path/seed=100+foobar=20+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "~/more=path/+blah/seed=100+foobar=20+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "\"whatever+=/\""}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  // missing extension
  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"foobar", "20"},
      {"seed", "100"}
   })) == "foobar=20+seed=100+_hash=asdf"
  );


}
