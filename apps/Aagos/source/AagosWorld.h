#ifndef AAGOS_WORLD_H
#define AAGOS_WORLD_H

#include "Evolve/World.h"

#include "AagosOrg.h"

class AagosWorld : public emp::World<AagosOrg> {
private:
public:
  AagosWorld() {
  }
  ~AagosWorld() { ; }
};

#endif
