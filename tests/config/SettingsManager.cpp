/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>

#include "emp/config/SettingsManager.hpp"

TEST_CASE("Test SettingsManager", "[config]")
{
  // AddSetting registers settings; HasSetting and Get work for all types
  {
    emp::SettingsManager cfg;
    int i = 3;
    double d = 1.5;
    bool b = true;
    emp::String s = "hello";
    size_t n = 10;

    cfg.AddSetting("i-val", i, "an int",    'i')
       .AddSetting("d-val", d, "a double")
       .AddSetting("b-val", b, "a bool")
       .AddSetting("s-val", s, "a string")
       .AddSetting("n-val", n, "a size_t");

    REQUIRE(cfg.HasSetting("i-val"));
    REQUIRE(cfg.HasSetting("d-val"));
    REQUIRE(cfg.HasSetting("b-val"));
    REQUIRE(cfg.HasSetting("s-val"));
    REQUIRE(cfg.HasSetting("n-val"));
    REQUIRE(!cfg.HasSetting("missing"));

    REQUIRE(cfg.Get<int>("i-val")         == 3);
    REQUIRE(cfg.Get<double>("d-val")      == 1.5);
    REQUIRE(cfg.Get<bool>("b-val")        == true);
    REQUIRE(cfg.Get<emp::String>("s-val") == "hello");
    REQUIRE(cfg.Get<size_t>("n-val")      == 10);

    REQUIRE(cfg.GetDesc("i-val")   == "an int");
    REQUIRE(cfg.GetFlag("i-val")   == 'i');
  }

  // Set() updates both the internal value and the bound variable
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    cfg.Set("x", 42);
    REQUIRE(cfg.Get<int>("x") == 42);
    REQUIRE(x == 42);
  }

  // Load from stream: basic assignment of each supported type
  {
    emp::SettingsManager cfg;
    int i = 0;
    double d = 0.0;
    emp::String s = "";
    size_t n = 0;

    cfg.AddSetting("i", i, "int")
       .AddSetting("d", d, "double")
       .AddSetting("s", s, "string")
       .AddSetting("n", n, "size_t");

    std::istringstream is(
      "i = 7;\n"
      "d = 2.5;\n"
      "s = \"hello world\";\n"
      "n = 100;\n"
    );
    REQUIRE(cfg.Load(is));
    REQUIRE(!cfg.HasError());
    REQUIRE(i == 7);
    REQUIRE(d == 2.5);
    REQUIRE(s == "hello world");
    REQUIRE(n == 100);
  }

  // Load: boolean literals (On/Off/True/False/1/0, case-insensitive)
  {
    emp::SettingsManager cfg;
    // cfg.SetVerbose();
    bool on_var = false, tr = false, off_var = true, fa = true, one = false, zero = true;

    cfg.AddSetting("on_v",  on_var,  "bool")
       .AddSetting("tr",    tr,      "bool")
       .AddSetting("off_v", off_var, "bool")
       .AddSetting("fa",    fa,      "bool")
       .AddSetting("one",   one,     "bool")
       .AddSetting("zero",  zero,    "bool");

    std::istringstream is("on_v = On\ntr = True\noff_v = Off\nfa = False\none = 1\nzero = 0\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(on_var  == true);
    REQUIRE(tr      == true);
    REQUIRE(off_var == false);
    REQUIRE(fa      == false);
    REQUIRE(one     == true);
    REQUIRE(zero    == false);
  }

  // Load: setting-to-setting copy (identifier on right-hand side)
  {
    emp::SettingsManager cfg;
    int a = 99, b = 0;
    cfg.AddSetting("a", a, "source")
       .AddSetting("b", b, "dest");

    std::istringstream is("b = a\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(b == 99);
  }

  // Load: comments and blank lines are skipped
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    std::istringstream is(
      "# this is a comment\n"
      "\n"
      "x = 7  # inline comment\n"
    );
    REQUIRE(cfg.Load(is));
    REQUIRE(x == 7);
  }

  // Load: multiple loads accumulate (second load overrides first)
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    std::istringstream is1("x = 10\n");
    REQUIRE(cfg.Load(is1));
    REQUIRE(x == 10);

    std::istringstream is2("x = 20\n");
    REQUIRE(cfg.Load(is2));
    REQUIRE(x == 20);
  }

  // AddKeyword: keyword fires callback with remaining line tokens
  {
    emp::SettingsManager cfg;
    emp::String captured = "";

    cfg.AddKeyword("greet", [&captured](emp::vector<emp::String> args) {
      captured = args.empty() ? "" : args[0];
    }, "test keyword");

    REQUIRE(cfg.HasKeyword("greet"));
    REQUIRE(cfg.HasIdentifier("greet"));
    REQUIRE(!cfg.HasSetting("greet"));

    std::istringstream is("greet world\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(captured == "world");
  }

  // Scopes: dot-notation registers and loads scoped settings
  {
    emp::SettingsManager cfg;
    int s1 = 0, s2 = 0;
    cfg.AddSetting("robot1.speed", s1, "robot1 speed")
       .AddSetting("robot2.speed", s2, "robot2 speed");

    std::istringstream is("robot1.speed = 10\nrobot2.speed = 20\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(s1 == 10);
    REQUIRE(s2 == 20);
  }

  // Scopes: brace-block syntax is equivalent to dot notation
  {
    emp::SettingsManager cfg;
    int s1 = 0, s2 = 0;
    cfg.AddSetting("robot1.speed", s1, "robot1 speed")
       .AddSetting("robot2.speed", s2, "robot2 speed");

    std::istringstream is("robot1 { speed = 10; }\nrobot2 { speed = 20; }\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(s1 == 10);
    REQUIRE(s2 == 20);
  }

  // Scopes: dot notation and brace syntax can be mixed in the same file
  {
    emp::SettingsManager cfg;
    int s1 = 0, s2 = 0;
    cfg.AddSetting("robot1.speed", s1, "robot1 speed")
       .AddSetting("robot2.speed", s2, "robot2 speed");

    std::istringstream is("robot1.speed = 5\nrobot2 { speed = 15; }\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(s1 == 5);
    REQUIRE(s2 == 15);
  }

  // Scopes: nested brace blocks
  {
    emp::SettingsManager cfg;
    int val = 0;
    cfg.AddSetting("outer.inner.val", val, "nested value");

    std::istringstream is("outer { inner { val = 99; } }\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(val == 99);
  }

  // Error: unknown identifier returns false and sets error
  {
    emp::SettingsManager cfg;
    std::istringstream is("unknown_setting = 5\n");
    REQUIRE(!cfg.Load(is));
    REQUIRE(cfg.HasError());
    REQUIRE(cfg.GetError().size() > 0);
  }

  // Error: load nonexistent file returns false and sets error
  {
    emp::SettingsManager cfg;
    REQUIRE(!cfg.Load("this_file_does_not_exist_12345.cfg"));
    REQUIRE(cfg.HasError());
  }

  // LoadArgs: -s / --set applies a bulk config string
  {
    emp::SettingsManager cfg;
    int x = 0;
    double d = 0.0;
    cfg.AddSetting("x", x, "int")
       .AddSetting("d", d, "double");

    emp::vector<emp::String> args = { "program", "-s", "x = 7; d = 3.5" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(x == 7);
    REQUIRE(d == 3.5);

    emp::vector<emp::String> args2 = { "program", "--set", "x = 99" };
    REQUIRE(cfg.LoadArgs(args2));
    REQUIRE(x == 99);
  }

  // LoadArgs: per-setting short flag sets the bound variable
  {
    emp::SettingsManager cfg;
    int count = 0;
    bool verbose = false;
    emp::String name = "";
    cfg.AddSetting("count",   count,   "count",   'c')
       .AddSetting("verbose", verbose, "verbose", 'v')
       .AddSetting("name",    name,    "name",    'n');

    emp::vector<emp::String> args = { "program", "-c", "5", "-v", "On", "-n", "Alice" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(count   == 5);
    REQUIRE(verbose == true);
    REQUIRE(name    == "Alice");
  }

  // LoadArgs: per-setting long option sets the bound variable
  {
    emp::SettingsManager cfg;
    int count = 0;
    emp::String name = "";
    cfg.AddSetting("count", count, "count", '\0')
       .AddSetting("name",  name,  "name",  '\0');

    emp::vector<emp::String> args = { "program", "--count", "12", "--name", "Bob" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(count == 12);
    REQUIRE(name  == "Bob");
  }

  // LoadArgs: unrecognised arguments are left untouched
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int", 'x');

    emp::vector<emp::String> args = { "program", "--other", "stuff", "-x", "3" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(x == 3);
    REQUIRE(args.size() == 5); // untouched without erase_on_use
  }

  // LoadArgs: erase_on_use removes processed flag+value pairs
  {
    emp::SettingsManager cfg;
    int x = 0;
    emp::String name = "";
    cfg.AddSetting("x",    x,    "int",    'x')
       .AddSetting("name", name, "string", 'n');

    emp::vector<emp::String> args = { "program", "-x", "7", "other", "-n", "Carol" };
    REQUIRE(cfg.LoadArgs(args, /*erase_on_use=*/true));
    REQUIRE(x    == 7);
    REQUIRE(name == "Carol");
    // Only "program" and "other" should remain
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "program");
    REQUIRE(args[1] == "other");
  }

  // LoadArgs: erase_on_use removes -s / --set and its config string
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    emp::vector<emp::String> args = { "program", "-s", "x = 4", "leftover" };
    REQUIRE(cfg.LoadArgs(args, /*erase_on_use=*/true));
    REQUIRE(x == 4);
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "program");
    REQUIRE(args[1] == "leftover");
  }

  // LoadArgs: missing value after short flag returns false and sets error
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int", 'x');

    emp::vector<emp::String> args = { "program", "-x" };
    REQUIRE(!cfg.LoadArgs(args));
    REQUIRE(cfg.HasError());
  }

  // LoadArgs: missing value after long option returns false and sets error
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("xval", x, "int");

    emp::vector<emp::String> args = { "program", "--xval" };
    REQUIRE(!cfg.LoadArgs(args));
    REQUIRE(cfg.HasError());
  }

  // Load: single-quoted string literals
  {
    emp::SettingsManager cfg;
    emp::String s = "";
    cfg.AddSetting("s", s, "string");

    std::istringstream is("s = 'hello world'\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(s == "hello world");
  }

  // Load: line continuation (trailing backslash joins lines)
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    // The '\' at end of line is consumed; the value token follows on the next line
    std::istringstream is("x = \\\n7\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(x == 7);
  }

  // Save: stream overload writes description comments and key=value lines
  {
    emp::SettingsManager cfg;
    int x = 42;
    bool b = true;
    cfg.AddSetting("x", x, "the answer")
       .AddSetting("b", b, "a flag");

    std::ostringstream os;
    REQUIRE(cfg.Save(os));
    const std::string out = os.str();
    REQUIRE(out.find("# the answer\n") != std::string::npos);
    REQUIRE(out.find("x = 42;\n")      != std::string::npos);
    REQUIRE(out.find("# a flag\n")     != std::string::npos);
    REQUIRE(out.find("b = On;\n")      != std::string::npos);
  }

  // Save: scoped settings are saved with their full dotted key
  {
    emp::SettingsManager cfg;
    int speed = 10;
    cfg.AddSetting("robot.speed", speed, "speed");

    std::ostringstream os;
    REQUIRE(cfg.Save(os));
    REQUIRE(os.str().find("robot.speed = 10;\n") != std::string::npos);
  }

  // AddKeyword: callback receives all argument tokens
  {
    emp::SettingsManager cfg;
    emp::vector<emp::String> captured;
    cfg.AddKeyword("cmd", [&captured](emp::vector<emp::String> args) {
      captured = args;
    }, "multi-arg keyword");

    std::istringstream is("cmd foo bar baz\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(captured.size() == 3);
    REQUIRE(captured[0] == "foo");
    REQUIRE(captured[1] == "bar");
    REQUIRE(captured[2] == "baz");
  }

  // LoadArgs: erase_on_use removes processed long option and its value
  {
    emp::SettingsManager cfg;
    int count = 0;
    cfg.AddSetting("count", count, "count");

    emp::vector<emp::String> args = { "program", "--count", "7", "other" };
    REQUIRE(cfg.LoadArgs(args, /*erase_on_use=*/true));
    REQUIRE(count == 7);
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "program");
    REQUIRE(args[1] == "other");
  }

  // LoadArgs: missing config string after --set / -s returns false and sets error
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    emp::vector<emp::String> args = { "program", "--set" };
    REQUIRE(!cfg.LoadArgs(args));
    REQUIRE(cfg.HasError());
  }

  // LoadArgs: scoped setting via long option (--robot.speed 42)
  {
    emp::SettingsManager cfg;
    int speed = 0;
    cfg.AddSetting("robot.speed", speed, "speed");

    emp::vector<emp::String> args = { "program", "--robot.speed", "42" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(speed == 42);
  }

  // Error: a successful load clears the error from a previous failed load
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int");

    std::istringstream bad("unknown = 5\n");
    REQUIRE(!cfg.Load(bad));
    REQUIRE(cfg.HasError());

    std::istringstream good("x = 3\n");
    REQUIRE(cfg.Load(good));
    REQUIRE(!cfg.HasError());
    REQUIRE(x == 3);
  }

  // Save and reload: round-trip preserves all values
  {
    emp::SettingsManager cfg;
    int i = 42;
    bool b = true;
    emp::String s = "saved";

    cfg.AddSetting("i", i, "int")
       .AddSetting("b", b, "bool")
       .AddSetting("s", s, "string");

    const std::string tmp = "/tmp/emp_settings_manager_test.cfg";
    REQUIRE(cfg.Save(tmp));

    int i2 = 0;
    bool b2 = false;
    emp::String s2 = "";
    emp::SettingsManager cfg2;
    cfg2.AddSetting("i", i2, "int")
        .AddSetting("b", b2, "bool")
        .AddSetting("s", s2, "string");

    REQUIRE(cfg2.Load(tmp));
    REQUIRE(i2 == 42);
    REQUIRE(b2 == true);
    REQUIRE(s2 == "saved");
  }
}
