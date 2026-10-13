/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file examples/io/CodeFile.cpp
 * @brief Generate three independently built programs in the current directory.
 */

#include "emp/io/CPPFile.hpp"
#include "emp/io/JSFile.hpp"
#include "emp/io/PythonFile.hpp"

int main() {
  emp::CPPFile cpp("generated.cpp");
  cpp.AddHeader("Generated C++ example").Include("<iostream>");
  cpp.AddVarSetting("value", 42).AddCode("int main() {").IncIndent();
  cpp.AddCode("std::cout << ${value} << '\\n';").DecIndent().AddCode("}").Write();

  emp::PythonFile python("generated.py");
  python.AddHeader("Generated Python example").ImportFrom("math", "sqrt");
  python.AddVarSetting("value", 1764).AddCode("def answer():").IncIndent();
  python.AddCode("return int(sqrt(@{value}))").DecIndent().AddCode("");
  python.AddCode("print(answer())").Write();

  emp::JSFile js("generated.mjs");
  js.AddHeader("Generated JavaScript example").ImportFrom("node:assert", "{ strict as assert }");
  js.AddVarSetting("value", 42).AddCode("function answer() {").IncIndent();
  js.AddCode("return @{value};").DecIndent().AddCode("}");
  js.AddCodeBlock("assert.equal(answer(), 42);", "console.log(`${answer()}`);").Write();
}
