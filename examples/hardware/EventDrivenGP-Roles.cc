
#include <iostream>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"
#include "../../evo3/World.h"

// Things to do:
//  [ ] Define fitness function
//    [ ] Build a 'deme' and evaluate it.
//  [ ] Define mutation function.  // Making a table of bit set values to make affinity assignment easier.

struct Org {
  double fitness;
  emp::EventDrivenGP hardware;
  double GetFitness() const { return fitness; }
  bool DoMutate(emp::Random &) { return false; }
};

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  // Some convenient type aliases:
  using event_t = typename emp::EventDrivenGP::event_t;
  using affinity_t = emp::BitSet<8>;

  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  // Setup random number generator.
  emp::Ptr<emp::Random> random;
  random.New(emp::Random(2));

  emp::World<Org> world(random, "Distributed-Role-World");

  return 0;
}
