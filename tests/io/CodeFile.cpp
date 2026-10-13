/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file tests/io/CodeFile.cpp
 * @brief Test shared code-file operations and language-specific output.
 */

#include <cstdio>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <utility>

#include "emp/io/CPPFile.hpp"
#include "emp/io/JSFile.hpp"
#include "emp/io/PythonFile.hpp"
#include "third-party/Catch/single_include/catch2/catch.hpp"

TEMPLATE_TEST_CASE("Code files share fluent text operations",
                   "[io]",
                   emp::CPPFile,
                   emp::PythonFile,
                   emp::JSFile) {
  TestType file;
  STATIC_REQUIRE(std::is_same_v<decltype(file.AddCode("")), TestType &>);
  STATIC_REQUIRE(std::is_same_v<decltype(file.AppendCode("")), TestType &>);
  STATIC_REQUIRE(std::is_same_v<decltype(file.IncIndent()), TestType &>);
  STATIC_REQUIRE(std::is_same_v<decltype(file.Write()), TestType &>);

  file.SetVarSymbol("@").AddVarSetting("value", 4, 2);
  file.SetIndentSize(3).IncIndent().AppendCode("answer = ", "@{value}");
  file.AppendPadding(18, '.').AppendCode("!");
  file.IncIndent(1).AddCodeBlock("first", "", "third");
  file.DecIndent(100).AddCode("top");
  REQUIRE(file.GetCode().size() == 5);
  CHECK(file.GetCode()[0] == "   answer = 42....!");
  CHECK(file.GetCode()[1] == "    first");
  CHECK(file.GetCode()[2] == "    ");
  CHECK(file.GetCode()[3] == "    third");
  CHECK(file.GetCode()[4] == "top");

  file.AddRawCode("@{value}").AppendRawCode(" @{missing}");
  CHECK(file.GetCode().back() == "@{value} @{missing}");
  file.SetVarSymbol("").AddCode("@{missing}");
  CHECK(file.GetCode().back() == "@{missing}");

  file.Clear().AppendRawCode("raw");
  CHECK(file.GetCode().back() == "raw");
  file.Clear().AppendPadding(3, '.').AppendCode("done");
  CHECK(file.GetCode().back() == "...done");
}

TEMPLATE_TEST_CASE("Code files clear content and variables but retain formatting",
                   "[io]",
                   emp::CPPFile,
                   emp::PythonFile,
                   emp::JSFile) {
  TestType file("old-file");
  file.AddHeader("old header").SetVarSymbol("@").AddVarSetting("value", 42);
  file.SetIndentSize(3).IncIndent().AddCode("@{value}");
  file.Clear();
  CHECK(file.GetFilename().empty());
  CHECK(file.GetCode().empty());
  std::ostringstream empty;
  file.Write(empty);
  CHECK(empty.str().empty());
  file.IncIndent().AddCode("@{value}");
  CHECK(file.GetCode().back() == "   @{value}");  // No old variable map remains.
  file.AddVarSetting("value", 7).AddCode("@{value}");
  CHECK(file.GetCode().back() == "   7");
}

TEMPLATE_TEST_CASE("Both code-file Write overloads use the concrete writer",
                   "[io]",
                   emp::CPPFile,
                   emp::PythonFile,
                   emp::JSFile) {
  TestType file;
  const emp::String filename = "temp/code-file-output.txt";
  file.SetFilename(filename).AddHeader("generated").AddCode("body");
  CHECK(file.GetFilename() == filename);
  std::ostringstream expected;
  CHECK(&file.Write(expected) == &file);
  CHECK(&file.Write() == &file);
  std::ifstream input(filename);
  REQUIRE(input.is_open());
  std::ostringstream actual;
  actual << input.rdbuf();
  CHECK(actual.str() == expected.str());
  input.close();
  CHECK(std::remove(filename.c_str()) == 0);
}

TEMPLATE_TEST_CASE("Code files remain independently copyable and movable",
                   "[io]",
                   emp::CPPFile,
                   emp::PythonFile,
                   emp::JSFile) {
  TestType original("original");
  original.SetVarSymbol("@").AddVarSetting("value", 1).AddCode("@{value}");
  TestType copy(original);
  copy.AddVarSetting("value", 2).AddCode("@{value}");
  CHECK(original.GetCode().size() == 1);
  CHECK(copy.GetCode().back() == "2");
  TestType moved(std::move(copy));
  CHECK(moved.GetFilename() == "original");
  CHECK(moved.GetCode().size() == 2);
  TestType assigned;
  assigned = moved;
  assigned = TestType("replacement");
  CHECK(assigned.GetCode().empty());
  CHECK(assigned.GetFilename() == "replacement");
}

TEST_CASE("CPPFile preserves includes, guards, namespace, and output indentation", "[io]") {
  emp::CPPFile file;
  file.AddHeader("Generated file").SetGuards("GENERATED_HPP").SetNamespace("example");
  file.Include("<vector>").Include("local.hpp").Include("<algorithm>");
  file.Include("\"local.hpp\"").Include("");
  file.AddVarSetting("value", 42).AddCode("int answer = ${value};");
  file.IncIndent().AddCodeBlock("// indented", " ").DecIndent();
  REQUIRE(file.GetIncludes().size() == 3);
  std::ostringstream output;
  file.Write(output);
  CHECK(output.str() == "// Generated file\n\n"
                        "#ifndef GENERATED_HPP\n#define GENERATED_HPP\n\n"
                        "#include \"local.hpp\"\n#include <algorithm>\n#include <vector>\n\n"
                        "namespace example {\n"
                        "  int answer = 42;\n    // indented\n\n"
                        "} // End of namespace example\n"
                        "#endif // #ifndef GENERATED_HPP\n");

  // Rendering does not mutate the stored text or accumulate indentation.
  std::ostringstream again;
  file.Write(again);
  CHECK(again.str() == output.str());
  CHECK(file.GetCode()[0] == "int answer = 42;");

  file.Clear().AddCode("int x;");
  CHECK(file.GetIncludes().empty());
  std::ostringstream plain;
  file.Write(plain);
  CHECK(plain.str() == "  int x;\n");
}

TEST_CASE("CPPFile retains quote-aware substitution and allows customization", "[io]") {
  emp::CPPFile file;
  file.AddVarSetting("value", 42);
  file.AddCode("const char * text = \"${value}\"; // ${value}");
  CHECK(file.GetCode().back() == "const char * text = \"${value}\"; // 42");
  file.AddCode("int x = ").AppendCode("${value};");
  CHECK(file.GetCode().back() == "int x = 42;");
  file.SetVarSymbol("@").SetVarQuotes("");
  file.AddCode("\"@{value}\"");
  CHECK(file.GetCode().back() == "\"42\"");
}

TEST_CASE("PythonFile emits ordered imports and explicit Python indentation", "[io]") {
  emp::PythonFile file;
  file.AddHeader("Generated Python").ImportFrom("__future__", "annotations");
  file.Import("sys").Import("os", "operating_system").Import("sys");
  file.ImportFrom("math", "sqrt as root, pi").ImportFrom("math", "sqrt as root, pi");
  file.AddVarSetting("value", 9).AddCode("def answer():").IncIndent();
  file.AddCode("return root(@{value})").AddCode("").DecIndent();
  file.AddCode("print(answer())");
  REQUIRE(file.GetImports().size() == 4);
  std::ostringstream output;
  file.Write(output);
  CHECK(output.str() ==
        "# Generated Python\n\n"
        "from __future__ import annotations\nimport sys\nimport os as operating_system\n"
        "from math import sqrt as root, pi\n\n"
        "def answer():\n    return root(9)\n\nprint(answer())\n");

  file.AddCode("message = '@{value}'");
  CHECK(file.GetCode().back() == "message = '9'");
  file.Clear().AddCode("pass");
  CHECK(file.GetImports().empty());
  std::ostringstream plain;
  file.Write(plain);
  CHECK(plain.str() == "pass\n");
}

TEST_CASE("JSFile supports module import forms without altering template expressions", "[io]") {
  emp::JSFile file;
  file.AddHeader("Generated JavaScript").Import("./setup.mjs");
  file.ImportFrom("./helpers.mjs", "helper");
  file.ImportFrom("node:fs", "{ readFileSync as read }");
  file.ImportFrom("node:path", "* as path").Import("./setup.mjs");
  file.ImportFrom("./helpers.mjs", "helper");
  file.AddVarSetting("value", 42).AddCode("function answer() {").IncIndent();
  file.AddCode("return @{value};").DecIndent().AddCode("}");
  file.AddCode("const message = `result: ${answer()} (@{value})`;");
  REQUIRE(file.GetImports().size() == 4);
  std::ostringstream output;
  file.Write(output);
  CHECK(output.str() == "// Generated JavaScript\n\n"
                        "import \"./setup.mjs\";\nimport helper from \"./helpers.mjs\";\n"
                        "import { readFileSync as read } from \"node:fs\";\n"
                        "import * as path from \"node:path\";\n\n"
                        "function answer() {\n  return 42;\n}\n"
                        "const message = `result: ${answer()} (42)`;\n");

  file.Clear().AddCode("const x = 1;");
  CHECK(file.GetImports().empty());
  std::ostringstream plain;
  file.Write(plain);
  CHECK(plain.str() == "const x = 1;\n");
}

TEST_CASE("JSFile quotes module specifiers as JavaScript literals", "[io]") {
  emp::JSFile file;
  file.Import("./a\"b\\c\n\t.mjs");
  file.ImportFrom("./a'b.mjs", "value");
  std::ostringstream output;
  file.Write(output);
  CHECK(output.str() == "import \"./a\\\"b\\\\c\\u000a\\u0009.mjs\";\n"
                        "import value from \"./a'b.mjs\";\n\n");
}
