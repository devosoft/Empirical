#include "tools/attrs.h"
#include <array>
#include <iostream>

using namespace emp::tools;

DEFINE_ATTR(PopulationSize);
DEFINE_ATTR(GenerationLength);
DEFINE_ATTR(DefaultGenome);

constexpr auto DEFAULT{PopulationSize(100) + GenerationLength(100) +
                       DefaultGenome("asdfferaefadfe")};

// Notice that this function has a different ordering than DEFAULT
void print(const std::string& name,
           const Attrs<typename PopulationSize::value_t<size_t>,
                       typename DefaultGenome::value_t<std::string>,
                       typename GenerationLength::value_t<size_t>>& args) {
  std::cout << name << " = " << args << std::endl;
}

void printSubset(
  const std::string& name,
  const Attrs<typename PopulationSize::value_t<size_t>,
              typename GenerationLength::value_t<size_t>>& args) {
  std::cout << name << " = " << args << std::endl;
}

int main() {
  // Ok, so these objects will auto convert when we need them to. Ordering of
  // fields is not important
  // print("DEFAULT", DEFAULT);
  // Also, we can select for a subset of a attribute pack when we want to
  printSubset("DEFAULT [SUBSET]", DEFAULT);

  // demo adding/chaning data
  print("DEFAULT.SetAttribute(populationSize(10))",
        DEFAULT.SetAttribute(PopulationSize(10)));
  print("DEFAULT + populationSize(10)", DEFAULT + PopulationSize(10));

  Attrs<typename PopulationSize::value_t<size_t>,
        typename DefaultGenome::value_t<std::string>,  // Notice that this will
                                                       // be auto-converted
        typename GenerationLength::value_t<size_t>>
    user = DEFAULT;
  // Set a single member
  user.SetDefaultGenome("ASDEDFDFSA");

  // Set multiple members at a time
  user = PopulationSize(100) + GenerationLength(10);

  print("DEFAULT >> STDIN", user);

  //
  // Example of compile time-unfolded runtime reflection:
  std::cout << "Updating the whole USER settings:" << std::endl;
  user.Foreach([](auto& name, auto& value) {
    std::cout << "set " << name << " (" << value << "): ";
    std::cin >> value;
  });

  std::cout << "New user settings: " << user << std::endl;
}
