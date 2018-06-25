/// This is the world for OpenOrgs

#ifndef OPEN_WORLD_H
#define OPEN_WORLD_H

#include "Evolve/World.h"
#include "geometry/Surface.h"

#include "config.h"
#include "OpenOrg.h"

class OpenWorld : public emp::World<OpenOrg> {
private:
  static constexpr size_t TAG_WIDTH = 16;
  using hardware_t = OpenOrg::hardware_t;
  using program_t = hardware_t::Program;
  using prog_fun_t = hardware_t::Function;
  using prog_tag_t = hardware_t::affinity_t;
  using event_lib_t = hardware_t::event_lib_t;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using hw_state_t = hardware_t::State;
 
  using surface_t = emp::Surface<OpenOrg>;

  OpenWorldConfig & config;
  inst_lib_t inst_lib;
  event_lib_t event_lib;
  surface_t surface;
  size_t next_id;

  // double pop_pressure = 1.0;  // How much pressure before an organism dies? 

  std::unordered_map<size_t, emp::Ptr<OpenOrg>> id_map;


public:  
  OpenWorld(OpenWorldConfig & _config)
    : config(_config), inst_lib(), event_lib(),
      surface({config.WORLD_X(), config.WORLD_Y()}),
      next_id(1), id_map()
  {
    SetPopStruct_Grow(false); // Don't automatically delete organism when new ones are born.

    // Make sure that we are tracking organisms by their IDs.
    OnPlacement( [this](size_t pos){
      size_t id = next_id++;
      GetOrg(pos).GetBrain().SetTrait((size_t)OpenOrg::Trait::ORG_ID, id);
      id_map[id] = &GetOrg(pos);
    });
    OnOrgDeath( [this](size_t pos){ id_map.erase( GetOrg(pos).GetID() ); } );

    // Setup new instructions for the instruction set.
    inst_lib.AddInst("Vroom", [this](hardware_t & hw, const inst_t & inst) {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      emp::Angle facing = org_ptr->GetFacing();
      org_ptr->Translate( facing.GetPoint(1.0) );
    }, 1, "Move forward.");

    inst_lib.AddInst("SpinRight", [this](hardware_t & hw, const inst_t & inst) mutable {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      org_ptr->RotateDegrees(-5.0);
    }, 1, "Rotate -5 degrees.");

    inst_lib.AddInst("SpinLeft", [this](hardware_t & hw, const inst_t & inst) mutable {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      org_ptr->RotateDegrees(5.0);
    }, 1, "Rotate 5 degrees.");

    // On each update, run organisms and make sure they stay on the surface.
    OnUpdate([this](size_t){
      Process(5);
      for (auto & org : *this) {
        double x = org.GetCenter().GetX();
        double y = org.GetCenter().GetY();
        if (x < 0.0) x += config.WORLD_X();
        if (y < 0.0) y += config.WORLD_Y();
        if (x >= config.WORLD_X()) x -= config.WORLD_X();
        if (y >= config.WORLD_Y()) y -= config.WORLD_Y();
        org.SetCenter({x,y});
      }
    });

    // Setup a mutation function.
    SetMutFun( [this](OpenOrg & org, emp::Random & random){
      org.SetRadius(org.GetRadius() * random.GetDouble(0.96, 1.05));
      return 1;
    });

    // Initialize a populaton of random organisms.
    Inject(OpenOrg(inst_lib, event_lib, random_ptr), config.INIT_POP_SIZE());
    for (size_t i = 0; i < config.INIT_POP_SIZE(); i++) {
      double x = random_ptr->GetDouble(config.WORLD_X());
      double y = random_ptr->GetDouble(config.WORLD_Y());
      GetOrg(i).SetCenter({x,y});
      surface.AddBody(&GetOrg(i));
      GetOrg(i).GetBrain().SetProgram(GenerateRandomProgram());
    }
  }
  ~OpenWorld() { ; }

  surface_t & GetSurface() { return surface; }

  program_t GenerateRandomProgram() {
    program_t prog(&inst_lib);
    size_t fcnt = random_ptr->GetUInt(config.PROGRAM_MIN_FUN_CNT(), config.PROGRAM_MAX_FUN_CNT());
    for (size_t fID = 0; fID < fcnt; ++fID) {
      prog_fun_t new_fun;
      new_fun.affinity.Randomize(*random_ptr);
      size_t icnt = random_ptr->GetUInt(config.PROGRAM_MIN_INST_CNT(), config.PROGRAM_MAX_INST_CNT());
      for (size_t iID = 0; iID < icnt; ++iID) {
        new_fun.PushInst(random_ptr->GetUInt(prog.GetInstLib()->GetSize()),
                         random_ptr->GetInt(config.PROGRAM_MAX_ARG_VAL()),
                         random_ptr->GetInt(config.PROGRAM_MAX_ARG_VAL()),
                         random_ptr->GetInt(config.PROGRAM_MAX_ARG_VAL()),
                         prog_tag_t());
        new_fun.inst_seq.back().affinity.Randomize(*random_ptr);
      }
      prog.PushFunction(new_fun);
    }
    return prog;
  }

  /// Test if two bodies have collided and act accordingly if they have.
  bool TestPairCollision(OpenOrg & body1, OpenOrg & body2) {
    // if (body1.IsLinked(body2)) return false;  // Linked bodies can overlap.

    const emp::Point dist = body1.GetCenter() - body2.GetCenter();
    const double sq_pair_dist = dist.SquareMagnitude();
    const double radius_sum = body1.GetRadius() + body2.GetRadius();
    const double sq_min_dist = radius_sum * radius_sum;

    // If there was no collision, return false.
    if (sq_pair_dist >= sq_min_dist) { return false; }

    // If we made it this far, there was a collision!

    return true;
  }
};

#endif
