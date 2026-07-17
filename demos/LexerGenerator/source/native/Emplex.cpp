/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024-2026.
*/
/**
 *  @file
 *  @brief Emplex: command-line lexer generator built on emp::Lexer.
 */

#include <fstream>
#include <istream>
#include <ostream>
#include <set>

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/config/command_line.hpp"
#include "emp/io/CPPFile.hpp"
#include "emp/io/File.hpp"
#include "emp/tools/String.hpp"

void LoadTokens(emp::Lexer & lexer, emp::String filename, bool verbose) {
  // emp::File silently produces an empty file if the load fails, so test the file first.
  emp::notify::TestError(!std::ifstream(filename).good(),
                         "Unable to open token definition file '", filename, "'.");

  emp::File file(filename);
  file.RemoveIfBegins("#");  // Remove all lines that are comments

  std::set<emp::String> used_names;  // Token names already loaded (to detect duplicates).

  for (emp::String line : file) {
    if (line.OnlyWhitespace()) continue;
    bool ignore = line.PopIf('-');
    emp::String name = line.PopWord();  // First entry on a line is the token name.
    emp::String regex = line.Trim();    // Regex is remainder, minus start & end whitespace.

    emp::notify::TestError(!regex.size(), "Token '", name, "' does not have an associated regex.");
    emp::notify::TestError(used_names.contains(name),
                           "Multiple token definitions named '", name, "'.");
    used_names.insert(name);

    if (ignore) lexer.IgnoreToken(name, regex);
    else lexer.AddToken(name, regex);

    if (verbose) {
      emp::notify::Message("Added token '", name, "'; ignore=", ignore, "; regex: ", regex);
    }
  }

  emp::notify::TestError(lexer.GetNumTokens() == 0,
                         "No token definitions found in '", filename, "'.");
}


int main(int argc, char* argv[])
{
  emp::vector<emp::String> args = emp::cl::ArgsToStrings(argc, argv);
  const bool help = emp::cl::UseArg(args, "-h");
  const bool verbose = emp::cl::UseArg(args, "-v");

  emp::String class_name("Lexer");
  int result = emp::cl::UseArgValue(args, "-c", class_name);
  emp::notify::TestError(result == -1, "The -c option must be followed by a class name.");

  emp::String dfa_name("DFA");
  result = emp::cl::UseArgValue(args, "-d", dfa_name);
  emp::notify::TestError(result == -1, "The -d option must be followed by a class name.");

  emp::String out_filename("lexer.hpp");
  result = emp::cl::UseArgValue(args, "-f", out_filename);
  emp::notify::TestError(result == -1, "The -f option must be followed by a filename.");

  emp::String inc_guards("__AUTOMATED_LEXER__");
  result = emp::cl::UseArgValue(args, "-g", inc_guards);
  emp::notify::TestError(result == -1, "The -g option must be followed by include guard name.");

  emp::String name_space("emplex");
  result = emp::cl::UseArgValue(args, "-n", name_space);
  emp::notify::TestError(result == -1, "The -n option must be followed by a namespace.");

  emp::String token_name("Token");
  result = emp::cl::UseArgValue(args, "-t", token_name);
  emp::notify::TestError(result == -1, "The -t option must be followed by a class name.");

  if (args.size() != 2 || help) {
    std::cerr << "Usage: " << args[0] << " {options} [config_file]\n"
      << "  The config_file should consist of a list of token definitions, one per line.\n"
      << "  Token definitions are a token name followed by a regular expression, e.g.:\n"
      << "    integer  [0-9]+\n"
      << "  Tokens with names starting with a minus sign are consumed and ignored, e.g.:\n"
      << "    -whitespace  [ \\t\\n\\r]+\n"
      << "  Lines beginning with a hash (#) are treated as comments.\n"
      << "  Options are:\n"
      << "    -c [class_name]  Set the name of generated lexer CLASS (default: " << class_name << ")\n"
      << "    -d [class_name]  Set the name of the generated DFA class (default: " << dfa_name << ")\n"
      << "    -f [filename]    Specify output FILENAME (default: " << out_filename << ")\n"
      << "    -g [guard_name]  Set the include GUARDS to use (default: " << inc_guards << ")\n"
      << "    -h               Print HELP (this message)\n"
      << "    -n [namespace]   Set NAMESPACE for generated code (default: " << name_space << ")\n"
      << "    -t [class_name]  Set the name of the generated Token class (default: " << token_name << ")\n"
      << "    -v               Print VERBOSE output\n"
      << std::endl;
    exit(help ? 0 : 1);  // Requested help is not an error.
  }

  emp::Lexer lexer;
  LoadTokens(lexer, args[1], verbose);

  emp::CPPFile file(out_filename);
  file.SetGuards(inc_guards);
  file.SetNamespace(name_space);
  lexer.WriteCPP(file, class_name, dfa_name, token_name);
  file.Write();

  return 0;
}
