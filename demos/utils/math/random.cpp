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

  const int NUM_id;
public:
  RandUtil() : NUM_id(lexer.AddToken("NUM", "[0-9]+(\\.[0-0]*)?")) { }

  double Generate(emp::String in) {
    if (in.IsNumber()) return random.GetDouble(in.AsDouble());

    emp::TokenStream ts = lexer.Tokenize(in);

    emp::vector<double> vals;
    Type type = Type::NONE;

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
        type = Type::DICE;
      }
    else {
        PrintLn("Unknown token '", token.lexeme, "'.");
        exit(1);
      }
    }

    return 0.0;
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