#include <fstream>
#include <istream>
#include <ostream>

#include "emp/base/vector.hpp"
#include "emp/base/notify.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/config/command_line.hpp"
#include "emp/io/File.hpp"
#include "emp/io/CPPFile.hpp"

void LoadTokens(emp::Lexer & lexer, emp::String filename, bool verbose) {
  emp::File file(filename);
  file.RemoveIfBegins("#");  // Remove all lines that are comments

  for (emp::String line : file) {
    if (line.OnlyWhitespace()) continue;
    bool ignore = line.PopIf('-');
    emp::String name = line.PopWord();  // First entry on a line is the token name.
    emp::String regex = line.Trim();    // Regex is remainder, minus start & end whitespace.

    emp::notify::TestError(!regex.size(), "Token '", name, "' does not have an associated regex.");

    if (ignore) lexer.IgnoreToken(name, regex);
    else lexer.AddToken(name, regex);

    if (verbose) {
      emp::notify::Message("Added token '", name, "'; ignore=", ignore, "; regex: ", regex);
    }

  }
}


int main(int argc, char* argv[])
{
  emp::vector<emp::String> args = emp::cl::ArgsToStrings(argc, argv);
  const bool help = emp::cl::UseArg(args, "-h");
  const bool verbose = emp::cl::UseArg(args, "-v");

  emp::String class_name("Lexer");
  int result = emp::cl::UseArgValue(args, "-c", class_name);
  emp::notify::TestError(result == -1, "The -c option must be followed by a class name.");

  emp::String out_filename("lexer.hpp");
  int result = emp::cl::UseArgValue(args, "-f", out_filename);
  emp::notify::TestError(result == -1, "The -f option must be followed by a filename.");

  emp::String inc_guards("__AUTOMATED_LEXER__");
  int result = emp::cl::UseArgValue(args, "-g", inc_guards);
  emp::notify::TestError(result == -1, "The -g option must be followed by include guard name.");

  emp::String name_space("emplex");
  int result = emp::cl::UseArgValue(args, "-n", name_space);
  emp::notify::TestError(result == -1, "The -n option must be followed by a namespace.");

  if (args.size() != 2 || help) {
    std::cerr << "Usage: " << args[0] << " {options} [config_file]\n"
      << "  The config_file should consist of a list of token definitions, on per line.\n"
      << "  Token definitions are a token name, a colon, and a regular expression, e.g.:\n"
      << "    integer : [0-9]+\n"
      << "  Tokens with names starting with a minus sign are consumed and ignored, e.g.:\n"
      << "    -whitespace : [ \\t\\n\\r]+\n"
      << "  Options are:\n"
      << "    -c [class_name]  Set the name of generated CLASS (default: " << class_name << ")\n"
      << "    -f [filename]    Specify output FILENAME (default: " << out_filename << ")\n"
      << "    -f [guard_name]  Set the include GUARDS to use (default: " << inc_guards << ")\n"
      << "    -h               Print HELP (this message)\n"
      << "    -n [namespace]   Set NAMESPACE for generated code (default: " << name_space << ")\n"
      << "    -v               Print VERBOSE output\n"
      << std::endl;
    exit(1);
  }

  emp::Lexer lexer;
  LoadTokens(lexer, args[1], verbose);

  emp::CPPFile file(out_filename);
  file.SetGuards(inc_guards);
  file.SetNamespace(name_space);
  lexer.WriteCPP(file, class_name);
  file.Write();

  return 0;
}
