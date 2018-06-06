/// These are OpenWorld organisms.

#ifndef OPEN_ORG_H
#define OPEN_ORG_H

#include "geometry/Point2D.h"
#include "hardware/EventDrivenGP.h"

struct OpenOrg {
  size_t id;

  emp::EventDrivenGP brain;
  double energy;
  double size;
  double time_born;
  double time_dead;

  emp::Point position;
  emp::Point velocity;  

  static size_t NextID() {
    static size_t next_id = 0;
    return next_id++;
  }

  OpenOrg() : id (NextID()), brain(), energy(0.0), size(0.0)
	    , time_born(0.0), time_dead(-1.0), position(), velocity() { ; }
  OpenOrg(const OpenOrg &) = default;
  OpenOrg(OpenOrg &&) = default;
  ~OpenOrg() { ; }

  OpenOrg & operator=(const OpenOrg &) = default;
  OpenOrg & operator=(OpenOrg &&) = default;
};

#endif
