#include "../../../include/emp/compiler/Lexer.hpp"
#include "../../../include/emp/io/io_utils.hpp"
#include "../../../include/emp/math/Random.hpp"
#include "../../../include/emp/tools/String.hpp"

using emp::PrintLn;

class RandUtil {
private:
  emp::Random random;
  emp::Lexer lexer;

  enum class Type {
    NONE=0, RANGE, DICE
  };
  emp::vector<double> vals;
  Type type = Type::NONE;

  const int NUM_id;
public:
  RandUtil() : NUM_id(lexer.AddToken("NUM", "[0-9]+(\\.[0-0]*)?")) { }

  double Parse(emp::String in) {
    emp::TokenStream ts = lexer.Tokenize(in);

    vals.clear();
    type = Type::NONE;
    for (emp::Token token : ts) {
      if (token == NUM_id) { vals.push_back(token.lexeme.AsDouble()); }
      else if (token == ':') {
        if (type != Type::NONE && type != Type::RANGE) {
          PrintLn("Cannot mix types for a random generator");
          exit(1);
        }
        type = Type::RANGE;
      }
      else if (token == 'd') {
        if (type != Type::NONE) {
          PrintLn("Cannot mix types or have multiple d's for a dice generator");
          exit(1);
        }
        // If we start with a 'd', it means 1 die.
        if (vals.size() == 0) vals.push_back(1.0);
        type = Type::DICE;
      }
    else {
        PrintLn("Unknown token '", token.lexeme, "'.");
        exit(1);
      }
    }

    return 0.0;
  }

  double Generate(emp::String in) {
    if (in.IsNumber()) return random.GetDouble(in.AsDouble());

    Parse(in);
    switch (type) {
    case Type::DICE: {
      emp_assert(vals.size() == 2);
      size_t total = 0;
      for (int i = 0; i < vals[0]; ++i) {
        total += random.GetUInt32(static_cast<uint32_t>(vals[1])) + 1;
      }
      return total;
      break;
    }
    }

    return -1;
  }
  };

int main(int argc, char * argv[]) {
  if (argc == 1){
    PrintLn("Format: ", argv[0], " [range]");
    PrintLn("Where range can be:");
    PrintLn("  X     -> generate a uniform random float from 0 to X");
    PrintLn("  X:Y   -> generate a uniform random float from X to Y");
    PrintLn("  X:Y:Z -> generate a uniform random float from X to Z in increments of Y");
    PrintLn("  dX    -> generate a uniform random integer from 1 to X (roll a die)");
    PrintLn("  XdY   -> roll X Y-sided dice and sum the result.");
    exit(1);
  }

  RandUtil ru;
  for (size_t i = 1; i < argc; ++i) {
    PrintLn(ru.Generate(argv[i]));
  }
}