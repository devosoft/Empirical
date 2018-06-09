/// These are OpenWorld organisms.

#ifndef OPEN_ORG_H
#define OPEN_ORG_H

#include "geometry/Point2D.h"
#include "hardware/EventDrivenGP.h"

struct OpenOrg : public emp::CircleBody2D {
  static constexpr size_t TAG_WIDTH = 16;
  using base_t = emp::CircleBody2D;
  using hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using event_lib_t = hardware_t::event_lib_t;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using hw_state_t = hardware_t::State;

  enum class Trait { ORG_ID };

  size_t id;

  hardware_t brain;

  double energy;
  double time_born;
  double time_dead;

  /// Values calculated from the above:
  double basic_cost;   /// How much energy is spent each update?

  OpenOrg(inst_lib_t & inst_lib, event_lib_t & event_lib, emp::Ptr<emp::Random> random_ptr)
    : base_t({100.0,100.0,5.0})
    , id(0), brain(inst_lib, event_lib, random_ptr), energy(0.0)
	  , time_born(0.0), time_dead(-1.0)
  { ; }
  OpenOrg(const OpenOrg &) = default;
  OpenOrg(OpenOrg &&) = default;
  ~OpenOrg() { ; }

  OpenOrg & operator=(const OpenOrg &) = default;
  OpenOrg & operator=(OpenOrg &&) = default;

  void Setup(emp::WorldPosition pos, emp::Random & random) {
    // std::cerr << "Seting up at pos " << pos.GetIndex() << std::endl;
    SetMass(GetRadius() * GetRadius());
    basic_cost = GetMass() * 0.01;
    RotateDegrees(random.GetDouble(360.0));
  }


  void Process(size_t exe_count) {
    brain.Process(exe_count);
  };
};

#endif
