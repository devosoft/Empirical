#include <fstream>
#include <istream>
#include <ostream>

#include "emp/base/vector.hpp"
#include "emp/compiler/Lexer.hpp"
#include "emp/config/command_line.hpp"
#include "emp/io/CPPFile.hpp"

int main(int argc, char* argv[])
{
  emp::vector<emp::String> args = emp::cl::ArgsToStrings(argc, argv);
  const bool help = emp::cl::UseArg(args, "-h");
  const bool verbose = emp::cl::UseArg(args, "-v");

  if (args.size() != 2 || help) {
    std::cerr << "Usage: " << args[0] << " {options} [config_file]\n"
      << "  The config_file should consist of a list of token definitions, on per line.\n"
      << "  Token definitions are a token name, a colon, and a regular expression, e.g.:\n"
      << "    integer : [0-9]+\n"
      << "  Tokens with names starting with a minus sign are consumed and ignored, e.g.:\n"
      << "    -whitespace : [ \\t\\n\\r]+\n"
      << "  Options are:\n"
      << "    -h   Print HELP (this message)\n"
      << "    -v   Print VERBOSE output\n"
      << std::endl;
    exit(1);
  }

  return 0;
}
