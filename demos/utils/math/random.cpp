#include "../../../include/emp/io/io_utils.hpp"
#include "../../../include/emp/math/Random.hpp"
#include "../../../include/emp/tools/String.hpp"

int main(int argc, char * argv[]) {
  if (argc == 1){
    emp::PrintLn("Format: ", argv[0], " [range]");
    emp::PrintLn("Where range can be:");
    emp::PrintLn("  X     -> generate a uniform random float from 0 to X");
    emp::PrintLn("  X:Y   -> generate a uniform random float from X to Y");
    emp::PrintLn("  X:Y:Z -> generate a uniform random float from X to Z in increments of Y");
    emp::PrintLn("  dX    -> generate a uniform random integer from 1 to X (roll a die)");
    emp::PrintLn("  XdY   -> roll X Y-sided dice and sum the result.");
    exit(1);
  }
}