/// This is the world for OpenOrgs

#ifndef OPEN_WORLD_H
#define OPEN_WORLD_H

#include "Evolve/World.h"

#include "config.h"
#include "OpenOrg.h"

class OpenWorld : public emp::World<OpenOrg> {
private:
  OpenWorldConfig config;

  double width = 100.0;
  double height = 100.0;
  double pop_pressure = 1.0;  // How much pressure before an organism dies? 

public:  
};

#endif
