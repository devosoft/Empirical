/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cstdint>
#include <limits>
#include <sstream>

#include "emp/config/SettingsManager.hpp"

TEST_CASE("Test SettingsManager", "[config]")
{
  // AddSetting registers settings; HasSetting and Get work for all types
  {
    emp::SettingsManager cfg;
    int64_t  i = 3;
    double   d = 1.5;
    bool     b = true;
    emp::String s = "hello";
    uint64_t n = 10;

    cfg.AddSetting("i_val", i, "an int64_t", 'i')
       .AddSetting("d_val", d, "a double")
       .AddSetting("b_val", b, "a bool")
       .AddSetting("s_val", s, "a string")
       .AddSetting("n_val", n, "a uint64_t");

    REQUIRE(cfg.HasSetting("i_val"));
    REQUIRE(cfg.HasSetting("d_val"));
    REQUIRE(cfg.HasSetting("b_val"));
    REQUIRE(cfg.HasSetting("s_val"));
    REQUIRE(cfg.HasSetting("n_val"));
    REQUIRE(!cfg.HasSetting("missing"));

    REQUIRE(cfg.Get<int64_t>("i_val")     == 3);
    REQUIRE(cfg.Get<double>("d_val")      == 1.5);
    REQUIRE(cfg.Get<bool>("b_val")        == true);
    REQUIRE(cfg.Get<emp::String>("s_val") == "hello");
    REQUIRE(cfg.Get<uint64_t>("n_val")    == 10);

    REQUIRE(cfg.GetDesc("i_val")   == "an int64_t");
    REQUIRE(cfg.GetFlag("i_val")   == 'i');
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
    int64_t  i = 0;
    double   d = 0.0;
    emp::String s = "";
    uint64_t n = 0;

    cfg.AddSetting("i", i, "int64_t")
       .AddSetting("d", d, "double")
       .AddSetting("s", s, "string")
       .AddSetting("n", n, "uint64_t");

    std::istringstream is(
      "i = 7;\n"
      "d = 2.5;\n"
      "s = \"hello world\";\n"
      "n = 100;\n"
    );
    REQUIRE(cfg.Load(is));
    REQUIRE(i == 7);
    REQUIRE(d == 2.5);
    REQUIRE(s == "hello world");
    REQUIRE(n == 100);
  }

  // Load: signed numbers, flexible decimal notation, and scientific notation
  {
    emp::SettingsManager cfg;
    int64_t neg_int = 0;
    double neg_double = 0.0;
    double positive_exp = 0.0;
    double negative_exp = 0.0;
    double leading_decimal = 0.0;
    int64_t min_int = 0;
    int64_t max_int = 0;
    uint64_t max_uint = 0;

    cfg.AddSetting("neg_int", neg_int, "negative integer")
       .AddSetting("neg_double", neg_double, "negative double")
       .AddSetting("positive_exp", positive_exp, "positive exponent")
       .AddSetting("negative_exp", negative_exp, "negative exponent")
       .AddSetting("leading_decimal", leading_decimal, "leading decimal")
       .AddSetting("min_int", min_int, "minimum signed integer")
       .AddSetting("max_int", max_int, "maximum signed integer")
       .AddSetting("max_uint", max_uint, "maximum unsigned integer");

    std::istringstream is(
      "neg_int = -42\n"
      "neg_double = -3.5\n"
      "positive_exp = 6.02E+23\n"
      "negative_exp = -2.5e-4\n"
      "leading_decimal = -.5\n"
      "min_int = -9223372036854775808\n"
      "max_int = 9223372036854775807\n"
      "max_uint = 18446744073709551615\n"
    );
    REQUIRE(cfg.Load(is));
    REQUIRE(neg_int == -42);
    REQUIRE(neg_double == -3.5);
    REQUIRE(positive_exp == 6.02E+23);
    REQUIRE(negative_exp == -2.5e-4);
    REQUIRE(leading_decimal == -0.5);
    REQUIRE(min_int == std::numeric_limits<int64_t>::min());
    REQUIRE(max_int == std::numeric_limits<int64_t>::max());
    REQUIRE(max_uint == std::numeric_limits<uint64_t>::max());
  }

  // SaveCurrent and Load round-trip values serialized in signed and scientific notation
  {
    int64_t saved_int = -42;
    double saved_double = 1.0e100;
    emp::SettingsManager writer;
    writer.AddSetting("i", saved_int, "integer")
          .AddSetting("d", saved_double, "double");

    std::ostringstream os;
    REQUIRE(writer.SaveCurrent(os));

    int64_t loaded_int = 0;
    double loaded_double = 0.0;
    emp::SettingsManager reader;
    reader.AddSetting("i", loaded_int, "integer")
          .AddSetting("d", loaded_double, "double");

    std::istringstream is(os.str());
    REQUIRE(reader.Load(is));
    REQUIRE(loaded_int == saved_int);
    REQUIRE(loaded_double == saved_double);
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

  // Load: string-to-string copy uses the semantic value, not a quoted config literal
  {
    emp::SettingsManager cfg;
    emp::String source = "hello \"quoted\" \\\\ tab\tline\nbreak";
    emp::String dest;
    cfg.AddSetting("source", source, "source")
       .AddSetting("dest", dest, "dest");

    std::istringstream is("dest = source\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(dest == source);
  }

  // AddValue: fixed values are readable and usable on the right-hand side.
  {
    emp::SettingsManager cfg;
    double result = 0.0;
    emp::String label;
    cfg.AddValue("PI", 3.141592653589793, "circle constant")
       .AddValue("LABEL", "ready", "fixed string")
       .AddSetting("result", result, "result")
       .AddSetting("label", label, "label");

    REQUIRE(cfg.HasValue("PI"));
    REQUIRE(cfg.HasValue("LABEL"));
    REQUIRE(cfg.HasValue("result"));       // Every setting is also a value.
    REQUIRE(!cfg.HasSetting("PI"));
    REQUIRE(cfg.HasSetting("result"));
    REQUIRE(cfg.HasIdentifier("PI"));
    REQUIRE(cfg.Get<double>("PI") == 3.141592653589793);
    REQUIRE(cfg.Get<emp::String>("LABEL") == "ready");

    std::istringstream is("result = PI\nlabel = LABEL\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(result == 3.141592653589793);
    REQUIRE(label == "ready");
  }

  // AddValue: getter values are dynamic and are not evaluated during registration.
  {
    emp::SettingsManager cfg;
    int current_time = 10;
    int getter_calls = 0;
    int observed = 0;
    cfg.AddValue("time", [&]() {
      ++getter_calls;
      return current_time;
    }, "current time");
    cfg.AddSetting("observed", observed, "observed time");

    REQUIRE(getter_calls == 0);
    REQUIRE(cfg.Get<int>("time") == 10);
    REQUIRE(getter_calls == 1);
    current_time = 25;

    std::istringstream is("observed = time\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(observed == 25);
    REQUIRE(getter_calls == 2);
  }

  // AddValue: writable values may be assigned but remain hidden from setting output.
  {
    emp::SettingsManager cfg;
    int hidden = 1;
    int visible = 2;
    cfg.AddValue(
      "hidden",
      [&hidden]() { return hidden; },
      [&hidden](int value) { hidden = value; },
      "hidden writable value"
    );
    cfg.AddSetting("visible", visible, "visible setting");

    std::istringstream is("hidden = 7\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(hidden == 7);
    cfg.Set("hidden", 9);
    REQUIRE(hidden == 9);
    cfg.Set("hidden", "10");
    REQUIRE(hidden == 10);
    cfg.Set("hidden", std::string{"11"});
    REQUIRE(hidden == 11);

    std::ostringstream saved;
    REQUIRE(cfg.SaveCurrent(saved));
    REQUIRE(saved.str().find("visible") != std::string::npos);
    REQUIRE(saved.str().find("hidden") == std::string::npos);

    std::ostringstream settings_help;
    cfg.PrintSettings(settings_help);
    REQUIRE(settings_help.str().find("visible") != std::string::npos);
    REQUIRE(settings_help.str().find("hidden") == std::string::npos);

    std::ostringstream status;
    cfg.PrintStatus(status);
    REQUIRE(status.str().find("hidden") == std::string::npos);
  }

  // RHS lookup is lexical: nearest scope first, then parents, then global scope.
  {
    emp::SettingsManager cfg;
    int global_result = 0;
    int outer_result = 0;
    int inner_result = 0;
    int fallback_result = 0;
    cfg.AddValue("PI", 1)
       .AddValue("outer.PI", 2)
       .AddValue("outer.inner.PI", 3)
       .AddSetting("global_result", global_result, "global")
       .AddSetting("outer.result", outer_result, "outer")
       .AddSetting("outer.inner.result", inner_result, "inner")
       .AddSetting("other.result", fallback_result, "fallback");

    std::istringstream is(
      "global_result = PI\n"
      "outer { result = PI; inner { result = PI; } }\n"
      "other { result = PI; }\n"
    );
    REQUIRE(cfg.Load(is));
    REQUIRE(global_result == 1);
    REQUIRE(outer_result == 2);
    REQUIRE(inner_result == 3);
    REQUIRE(fallback_result == 1);
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

  // Global keywords may share non-leading components with scoped setting names
  {
    emp::SettingsManager cfg;
    int robot_speed = 0;
    emp::String captured;
    cfg.AddKeyword("speed", [&captured](emp::vector<emp::String> args) {
      captured = args.empty() ? "" : args[0];
    }, "global speed keyword");
    cfg.AddSetting("robot.speed", robot_speed, "robot speed");

    std::istringstream is("robot { speed = 42; }\nspeed fast\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(robot_speed == 42);
    REQUIRE(captured == "fast");

    // The reverse registration order should be valid as well.
    emp::SettingsManager reverse_cfg;
    int reverse_speed = 0;
    reverse_cfg.AddSetting("robot.speed", reverse_speed, "robot speed");
    reverse_cfg.AddKeyword("speed", [](emp::vector<emp::String>) {}, "global speed keyword");
    REQUIRE(reverse_cfg.HasSetting("robot.speed"));
    REQUIRE(reverse_cfg.HasKeyword("speed"));
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

  // LoadArgs: multiple long setting options are applied and removed from args
  {
    emp::SettingsManager cfg;
    int x = 0;
    double d = 0.0;
    cfg.AddSetting("x", x, "int")
       .AddSetting("d", d, "double");

    emp::vector<emp::String> args = { "program", "--x", "7", "--d", "3.5" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(x == 7);
    REQUIRE(d == 3.5);
    REQUIRE(args.size() == 1); // only "program" remains
  }

  // LoadArgs: per-setting short flag sets the bound variable and is removed from args
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
    REQUIRE(args.size() == 1); // only "program" remains
  }

  // LoadArgs: per-setting long option sets the bound variable and is removed from args
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
    REQUIRE(args.size() == 1); // only "program" remains
  }

  // LoadArgs: unrecognised arguments are left untouched
  {
    emp::SettingsManager cfg;
    int x = 0;
    cfg.AddSetting("x", x, "int", 'x');

    emp::vector<emp::String> args = { "program", "other", "stuff", "-x", "3" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(x == 3);
    REQUIRE(args.size() == 3); // "program", "other", "stuff" remain; "-x" and "3" consumed
  }

  // LoadArgs: processed short flags and their values are removed from args
  {
    emp::SettingsManager cfg;
    int x = 0;
    emp::String name = "";
    cfg.AddSetting("x",    x,    "int",    'x')
       .AddSetting("name", name, "string", 'n');

    emp::vector<emp::String> args = { "program", "-x", "7", "other", "-n", "Carol" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(x    == 7);
    REQUIRE(name == "Carol");
    REQUIRE(args.size() == 2); // only "program" and "other" remain
    REQUIRE(args[0] == "program");
    REQUIRE(args[1] == "other");
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
    REQUIRE(out.find("b = 1;\n")      != std::string::npos);
  }

  // Save: scoped settings are grouped into brace blocks
  {
    emp::SettingsManager cfg;
    int speed = 10;
    int count = 3;
    cfg.AddSetting("robot.speed", speed, "speed");
    cfg.AddSetting("robot.count", count, "count");

    std::ostringstream os;
    REQUIRE(cfg.Save(os));
    const std::string out = os.str();
    REQUIRE(out.find("robot {") != std::string::npos);
    REQUIRE(out.find("  speed = 10;\n") != std::string::npos);
    REQUIRE(out.find("  count = 3;\n") != std::string::npos);
    REQUIRE(out.find("}") != std::string::npos);
    // The full dotted key should NOT appear in scoped output
    REQUIRE(out.find("robot.speed") == std::string::npos);
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

  // LoadArgs: processed long option and its value are removed from args
  {
    emp::SettingsManager cfg;
    int count = 0;
    cfg.AddSetting("count", count, "count");

    emp::vector<emp::String> args = { "program", "--count", "7", "other" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(count == 7);
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "program");
    REQUIRE(args[1] == "other");
  }

  // LoadArgs: keyword triggered via short flag (-k arg1 arg2)
  {
    emp::SettingsManager cfg;
    emp::vector<emp::String> captured;
    cfg.AddKeyword("greet", [&captured](emp::vector<emp::String> args) {
      captured = args;
    }, "greeting keyword", 'g');

    emp::vector<emp::String> args = { "program", "-g", "hello", "world" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(captured.size() == 2);
    REQUIRE(captured[0] == "hello");
    REQUIRE(captured[1] == "world");
  }

  // LoadArgs: keyword triggered via long option (--keyword arg1 arg2)
  {
    emp::SettingsManager cfg;
    emp::vector<emp::String> captured;
    cfg.AddKeyword("greet", [&captured](emp::vector<emp::String> args) {
      captured = args;
    }, "greeting keyword");

    emp::vector<emp::String> args = { "program", "--greet", "hello", "world" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(captured.size() == 2);
    REQUIRE(captured[0] == "hello");
    REQUIRE(captured[1] == "world");
  }

  // LoadArgs: keyword max_args limits how many arguments are consumed
  {
    emp::SettingsManager cfg;
    emp::vector<emp::String> captured;
    cfg.AddKeyword("greet", [&captured](emp::vector<emp::String> args) {
      captured = args;
    }, "greeting keyword", '\0', /*max_args=*/1);

    emp::vector<emp::String> args = { "program", "--greet", "hello", "world" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(captured.size() == 1);
    REQUIRE(captured[0] == "hello");
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

  // AddSetting(getter, setter): primary use case – class with private member via accessors
  {
    class Robot {
      int64_t speed_ = 0;
    public:
      int64_t GetSpeed() const { return speed_; }
      void    SetSpeed(int64_t v) { speed_ = v; }
    };
    Robot robot;
    emp::SettingsManager cfg;
    cfg.AddSetting("speed",
      [&robot]() { return robot.GetSpeed(); },
      [&robot](int64_t v) { robot.SetSpeed(v); },
      "Robot speed", 'p');

    REQUIRE(cfg.HasSetting("speed"));
    cfg.Set("speed", INT64_C(42));
    REQUIRE(robot.GetSpeed() == 42);
    REQUIRE(cfg.Get<int64_t>("speed") == 42);
  }

  // AddSetting(getter, setter): setter with side effects (value clamping)
  {
    int val = 0;
    emp::SettingsManager cfg;
    cfg.AddSetting("level",
      [&val]() { return val; },
      [&val](int v) { val = std::clamp(v, 0, 10); },
      "Level (clamped 0-10)");

    std::istringstream is("level = 15\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(val == 10);  // clamped to max

    cfg.Set("level", -5);
    REQUIRE(val == 0);   // clamped to min
  }

  // AddSetting(getter, setter): Load updates value through setter; setter call is tracked
  {
    int x = 0;
    int set_count = 0;
    emp::SettingsManager cfg;
    cfg.AddSetting("x",
      [&x]() { return x; },
      [&x, &set_count](int v) { x = v; ++set_count; },
      "int with call counter");

    std::istringstream is("x = 7\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(x == 7);
    REQUIRE(set_count == 1);
  }

  // AddSetting(getter, setter): all supported types
  {
    emp::String  s = "";
    bool         b = false;
    double       d = 0.0;
    uint64_t     n = 0;
    emp::SettingsManager cfg;
    cfg.AddSetting("s", [&s]() { return s; }, [&s](emp::String v)  { s = v; }, "string")
       .AddSetting("b", [&b]() { return b; }, [&b](bool v)         { b = v; }, "bool")
       .AddSetting("d", [&d]() { return d; }, [&d](double v)        { d = v; }, "double")
       .AddSetting("n", [&n]() { return n; }, [&n](uint64_t v)      { n = v; }, "uint64_t");

    std::istringstream is("s = \"hello\"\nb = On\nd = 3.14\nn = 99\n");
    REQUIRE(cfg.Load(is));
    REQUIRE(s == "hello");
    REQUIRE(b == true);
    REQUIRE(d == 3.14);
    REQUIRE(n == 99);
  }

  // AddSetting(getter, setter): LoadArgs routes through the setter
  {
    int count = 0;
    emp::SettingsManager cfg;
    cfg.AddSetting("count",
      [&count]() { return count; },
      [&count](int v) { count = v; },
      "count", 'c');

    emp::vector<emp::String> args = { "program", "-c", "5" };
    REQUIRE(cfg.LoadArgs(args));
    REQUIRE(count == 5);
    REQUIRE(args.size() == 1);
  }

  // Save uses the registered default; SaveCurrent reads the live value through the getter.
  {
    int x = 99;
    emp::SettingsManager cfg;
    cfg.AddSetting("x",
      [&x]() { return x; },
      [&x](int v) { x = v; },
      "a value");

    // Save always writes the default captured at AddSetting time.
    std::ostringstream os;
    REQUIRE(cfg.Save(os));
    REQUIRE(os.str().find("x = 99;\n") != std::string::npos);

    x = 7;  // mutate directly, bypassing the setter
    std::ostringstream os2;
    REQUIRE(cfg.Save(os2));
    REQUIRE(os2.str().find("x = 99;\n") != std::string::npos);  // default unchanged

    // SaveCurrent captures the live value.
    std::ostringstream os3;
    REQUIRE(cfg.SaveCurrent(os3));
    REQUIRE(os3.str().find("x = 7;\n") != std::string::npos);
  }

  // AddSetting(getter, setter): getter can return a computed (non-stored) value
  {
    int a = 3, b = 4;
    int received = -1;
    emp::SettingsManager cfg;
    cfg.AddSetting("sum",
      [&a, &b]() { return a + b; },
      [&received](int v) { received = v; },
      "computed sum");

    REQUIRE(cfg.Get<int>("sum") == 7);
    a = 10;
    REQUIRE(cfg.Get<int>("sum") == 14);  // getter reflects live state
  }
}

TEST_CASE("Test SettingsManager SerialSave and SerialLoad", "[config][serialize]")
{
  // Basic round-trip for all supported types
  {
    int64_t  i = 7;
    uint64_t n = 100;
    double   d = 3.14;
    bool     b = true;
    emp::String s = "hello world";

    emp::SettingsManager cfg;
    cfg.AddSetting("i", i, "int64_t")
       .AddSetting("n", n, "uint64_t")
       .AddSetting("d", d, "double")
       .AddSetting("b", b, "bool")
       .AddSetting("s", s, "string");

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int64_t  i2 = 0;
    uint64_t n2 = 0;
    double   d2 = 0.0;
    bool     b2 = false;
    emp::String s2 = "";

    emp::SettingsManager cfg2;
    cfg2.AddSetting("i", i2, "int64_t")
        .AddSetting("n", n2, "uint64_t")
        .AddSetting("d", d2, "double")
        .AddSetting("b", b2, "bool")
        .AddSetting("s", s2, "string");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(i2 == 7);
    REQUIRE(n2 == 100);
    REQUIRE(d2 == 3.14);
    REQUIRE(b2 == true);
    REQUIRE(s2 == "hello world");
  }

  // SerialSave captures the live value, not the registered default
  {
    int x = 1;
    emp::SettingsManager cfg;
    cfg.AddSetting("x", x, "int");
    cfg.Set("x", 999);
    REQUIRE(x == 999);

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int x2 = 0;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("x", x2, "int");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(x2 == 999);
  }

  // Scoped (dotted) settings round-trip correctly
  {
    int speed = 10, count = 3;
    emp::SettingsManager cfg;
    cfg.AddSetting("robot.speed", speed, "speed")
       .AddSetting("robot.count", count, "count");

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int speed2 = 0, count2 = 0;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("robot.speed", speed2, "speed")
        .AddSetting("robot.count", count2, "count");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(speed2 == 10);
    REQUIRE(count2 == 3);
  }

  // Getter/setter-based settings round-trip correctly
  {
    int speed = 55;
    emp::SettingsManager cfg;
    cfg.AddSetting("speed",
      [&speed]() { return speed; },
      [&speed](int v) { speed = v; },
      "speed");

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int speed2 = 0;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("speed",
      [&speed2]() { return speed2; },
      [&speed2](int v) { speed2 = v; },
      "speed");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(speed2 == 55);
  }

  // String with special characters round-trips correctly
  {
    emp::String s = "say \"hi\"\tthere";
    emp::SettingsManager cfg;
    cfg.AddSetting("s", s, "string");

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    emp::String s2 = "";
    emp::SettingsManager cfg2;
    cfg2.AddSetting("s", s2, "string");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(s2 == s);
  }

  // Named values are not included in serialized setting state.
  {
    int setting = 42;
    int writable_value = 7;
    emp::SettingsManager cfg;
    cfg.AddSetting("setting", setting, "setting")
       .AddValue("constant", 100)
       .AddValue(
         "writable_value",
         [&writable_value]() { return writable_value; },
         [&writable_value](int value) { writable_value = value; }
       );

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int loaded_setting = 0;
    int loaded_value = -1;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("setting", loaded_setting, "setting")
        .AddValue("constant", 200)
        .AddValue(
          "writable_value",
          [&loaded_value]() { return loaded_value; },
          [&loaded_value](int value) { loaded_value = value; }
        );

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(loaded_setting == 42);
    REQUIRE(cfg2.Get<int>("constant") == 200);
    REQUIRE(loaded_value == -1);
  }

  // Settings absent from the pod keep their current values
  {
    int a = 10;
    emp::SettingsManager cfg;
    cfg.AddSetting("a", a, "a");  // "b" not registered here, so not serialized

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int a2 = 0, b2 = 77;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("a", a2, "a")
        .AddSetting("b", b2, "b");

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);

    REQUIRE(a2 == 10);  // restored
    REQUIRE(b2 == 77);  // not in pod; unchanged
  }

  // Settings in the pod but not registered emit a warning and are skipped
  // (expected warning: "SettingsManager::SerialLoad: setting 'x' not found; skipping.")
  {
    int x = 5;
    emp::SettingsManager cfg;
    cfg.AddSetting("x", x, "int");

    std::stringstream ss;
    emp::SerialPod save_pod(ss, true);
    cfg.SerialSave(save_pod);

    int y = 99;
    emp::SettingsManager cfg2;
    cfg2.AddSetting("y", y, "int");  // "x" from the pod is unknown

    emp::SerialPod load_pod(ss, false);
    cfg2.SerialLoad(load_pod);  // warns about "x", skips it

    REQUIRE(y == 99);  // unchanged
  }
}
