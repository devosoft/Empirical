/// These are OpenWorld organisms.

#ifndef OPEN_ORG_H
#define OPEN_ORG_H

#include "geometry/Point2D.h"
#include "hardware/EventDrivenGP.h"

class OpenOrg {
public:
  static constexpr size_t TAG_WIDTH = 16;
  using hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using event_lib_t = hardware_t::event_lib_t;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using hw_state_t = hardware_t::State;

  enum class Trait { ORG_ID };

private:
  size_t id;
  size_t color_id;
  hardware_t brain;

  emp::Point center;
  double radius;
  emp::Angle facing;

  double energy;
  double time_born;
  double time_dead;

  /// Values calculated from the above:
  double base_cost;   /// How much energy is spent each update?

public:
  OpenOrg(inst_lib_t & inst_lib, event_lib_t & event_lib, emp::Ptr<emp::Random> random_ptr)
    : id(0), color_id(0), brain(inst_lib, event_lib, random_ptr)
    , center(1.0, 1.0), radius(5.0), facing()
    , energy(0.0), time_born(0.0), time_dead(-1.0), base_cost(radius*radius*0.01)
  { ; }
  OpenOrg(const OpenOrg &) = default;
  OpenOrg(OpenOrg &&) = default;
  ~OpenOrg() { ; }

  OpenOrg & operator=(const OpenOrg &) = default;
  OpenOrg & operator=(OpenOrg &&) = default;

  size_t GetID() const { return id; }
  size_t GetColorID() const { return color_id; }
  hardware_t & GetBrain() { return brain; }
  const hardware_t & GetBrain() const { return brain; }
  emp::Point GetCenter() const { return center; }
  double GetRadius() const { return radius; }
  emp::Angle GetFacing() const { return facing; }
  double GetEnergy() const { return energy; }
  double GetTimeBorn() const { return time_born; }
  double GetTimeDead() const { return time_dead; }
  double GetBaseCost() const { return base_cost; }

  double GetMass() const { return radius * radius; }

  OpenOrg & SetID(size_t _in) { id = _in; return *this; }
  OpenOrg & GetColorID(size_t _in) { color_id = _in; return *this; }
  // OpenOrg & SetBrain(const hardware_t & _in) { brain = _in; return *this; }
  OpenOrg & SetCenter(emp::Point _in) { center = _in; return *this; }
  OpenOrg & SetRadius(double _in) { radius = _in; return *this; }
  OpenOrg & SetFacing(emp::Angle _in) { facing = _in; return *this; }
  OpenOrg & SetEnergy(double _in) { energy = _in; return *this; }
  OpenOrg & SetTimeBorn(double _in) { time_born = _in; return *this; }
  OpenOrg & SetTimeDead(double _in) { time_dead = _in; return *this; }
  OpenOrg & SetBaseCost(double _in) { base_cost = _in; return *this; }

  OpenOrg & RotateDegrees(double degrees) { facing.RotateDegrees(degrees); return *this; }
  OpenOrg & Translate(emp::Point shift) { center += shift; return *this; }

  void Setup(emp::WorldPosition pos, emp::Random & random) {
    // std::cerr << "Seting up at pos " << pos.GetIndex() << std::endl;
    base_cost = GetMass() * 0.01;
    RotateDegrees(random.GetDouble(360.0));
  }

  void Process(size_t exe_count) {
    brain.Process(exe_count);
  };
};

#endif
