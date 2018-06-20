/// This is the world for OpenOrgs

#ifndef OPEN_WORLD_H
#define OPEN_WORLD_H

#include "Evolve/World.h"
#include "geometry/Surface2D.h"

#include "config.h"
#include "OpenOrg.h"

class OpenWorld : public emp::World<OpenOrg> {
private:
  static constexpr size_t TAG_WIDTH = 16;
  using hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using event_lib_t = hardware_t::event_lib_t;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using hw_state_t = hardware_t::State;
 
  using surface_t = emp::Surface2D<emp::CircleBody2D>;

  OpenWorldConfig & config;
  inst_lib_t inst_lib;
  event_lib_t event_lib;
  surface_t surface;

  // double pop_pressure = 1.0;  // How much pressure before an organism dies? 

  std::unordered_map<size_t, emp::Ptr<OpenOrg>> id_map;

public:  
  OpenWorld(OpenWorldConfig & _config)
    : config(_config), inst_lib(), event_lib(), surface(config.WORLD_X(), config.WORLD_Y())
  {
    OnPlacement( [this](size_t pos){ id_map[ GetOrg(pos).id ] = &GetOrg(pos); } );
    OnOrgDeath( [this](size_t pos){ id_map.erase( GetOrg(pos).id ); } );

    inst_lib.AddInst("Vroom", [this](hardware_t & hw, const inst_t & inst) {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      emp::Angle facing = org_ptr->body.GetOrientation();
      facing.GetPoint(10.0);
    }, 1, "Move forward.");

    Inject(OpenOrg(inst_lib, event_lib, random_ptr), config.INIT_POP_SIZE());
    for (size_t i = 0; i < config.INIT_POP_SIZE(); i++) {
      double x = random_ptr->GetDouble(config.WORLD_X());
      double y = random_ptr->GetDouble(config.WORLD_Y());
      GetOrg(i).body.SetPosition({x,y});
      surface.AddBody(&GetOrg(i).body);
    }
  }
  ~OpenWorld() { ; }

  surface_t & GetSurface() { return surface; }
};

#endif
