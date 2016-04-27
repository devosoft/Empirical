#include <iostream>
#include <string>

#include "../../evo/World.h"
#include "../../tools/Random.h"

using namespace emp::evo;

class Org {
private:
  int value;

public:
  Org(int in_v) : value(in_v) { ; }

  double Fitness() const { return (double) value; }
  bool Mutate(emp::Random & r) { value += r.GetInt(-50,51); return true; }

  void Print() { std::cout << value; }
};

int main()
{
  emp::Random random;
  World<Org, PopEA> world(random, "loud");

  // world.pop.Config(10, 100);

  // world.OffspringReady( [&random](Org * org) { org->Print(); std::cout << " born!" << std::endl;});
  std::function<void(Org*)> fun = [&random](Org * org) { org->Print(); std::cout << " born!" << std::endl;};
  auto * act = new emp::Action<Org*>(fun);

  emp::PrintSignalNames();
  emp::LinkSignal( "loud::offspring-ready", act );

  for (int i = 0; i < 36; i++) world.Insert( random.GetInt(100, 1000) );

  // Print world
  for (int i = 0; i < world.GetSize(); i++) {
    world[i].Print();
    std::cout << " ";
  }
  std::cout << std::endl;

  for (int gen = 0; gen < 10; gen++) {
    world.EliteSelect(10, 3);
    world.TournamentSelect(5, 27);
    world.Update();

    // Print world
    std::cout << "GEN " << gen
	      << "  size=" << world.GetSize()
	      << std::endl;
    for (int i = 0; i < world.GetSize(); i++) {
      world[i].Print();
      std::cout << " ";
    }
    std::cout << std::endl;

  }
}
