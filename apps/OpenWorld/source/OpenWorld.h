/// This is the world for OpenOrgs

#ifndef OPEN_WORLD_H
#define OPEN_WORLD_H

#include "Evolve/World.h"
#include "geometry/Surface.h"
#include "hardware/signalgp_utils.h"
#include "tools/math.h"

#include "config.h"
#include "OpenOrg.h"
#include "OpenResource.h"

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
 
  using surface_t = emp::Surface<OpenOrg, OpenResource>;
  using mutator_t = emp::SignalGPMutator<TAG_WIDTH>;

  OpenWorldConfig & config;
  inst_lib_t inst_lib;
  event_lib_t event_lib;
  surface_t surface;
  size_t next_id;

  mutator_t signalgp_mutator;

  // double pop_pressure = 1.0;  // How much pressure before an organism dies? 

  std::unordered_map<size_t, emp::Ptr<OpenOrg>> id_map;

  emp::vector<OpenResource> resources;

public:  
  OpenWorld(OpenWorldConfig & _config)
    : config(_config), inst_lib(), event_lib(),
      surface({config.WORLD_X(), config.WORLD_Y()}),
      next_id(1), signalgp_mutator(), id_map()
  {
    SetPopStruct_Grow(false); // Don't automatically delete organism when new ones are born.

    // Setup organism to share parent's surface features.
    OnOffspringReady( [this](OpenOrg & org, size_t parent_pos){
      emp::Point parent_center = surface.GetCenter(GetOrg(parent_pos).GetSurfaceID());
      double parent_radius = surface.GetRadius(GetOrg(parent_pos).GetSurfaceID());
      size_t surface_id = surface.AddBody(&org, parent_center, parent_radius);
      org.SetSurfaceID(surface_id);
    });
    // Make sure that we are tracking organisms by their IDs once placed.
    OnPlacement( [this](size_t pos){ 
      size_t id = next_id++;
      GetOrg(pos).GetBrain().SetTrait((size_t)OpenOrg::Trait::ORG_ID, id);
      id_map[id] = &GetOrg(pos);
    } );
    OnOrgDeath( [this](size_t pos) {
      surface.RemoveBody(GetOrg(pos).GetSurfaceID());
      id_map.erase(GetOrg(pos).GetID());
    });

    // Setup SignalGP mutations.
    signalgp_mutator.SetProgMinFuncCnt(config.PROGRAM_MIN_FUN_CNT());
    signalgp_mutator.SetProgMaxFuncCnt(config.PROGRAM_MAX_FUN_CNT());
    signalgp_mutator.SetProgMinFuncLen(config.PROGRAM_MIN_FUN_LEN());
    signalgp_mutator.SetProgMaxFuncLen(config.PROGRAM_MAX_FUN_LEN());
    signalgp_mutator.SetProgMinArgVal(config.PROGRAM_MIN_ARG_VAL());
    signalgp_mutator.SetProgMaxArgVal(config.PROGRAM_MAX_ARG_VAL());
    signalgp_mutator.SetProgMaxTotalLen(config.PROGRAM_MAX_FUN_CNT() * config.PROGRAM_MAX_FUN_LEN());

    // Setup other SignalGP functions.
    signalgp_mutator.ARG_SUB__PER_ARG(config.ARG_SUB__PER_ARG());
    signalgp_mutator.INST_SUB__PER_INST(config.INST_SUB__PER_INST());
    signalgp_mutator.INST_INS__PER_INST(config.INST_INS__PER_INST());
    signalgp_mutator.INST_DEL__PER_INST(config.INST_DEL__PER_INST());
    signalgp_mutator.SLIP__PER_FUNC(config.SLIP__PER_FUNC());
    signalgp_mutator.FUNC_DUP__PER_FUNC(config.FUNC_DUP__PER_FUNC());
    signalgp_mutator.FUNC_DEL__PER_FUNC(config.FUNC_DEL__PER_FUNC());
    signalgp_mutator.TAG_BIT_FLIP__PER_BIT(config.TAG_BIT_FLIP__PER_BIT());

    // Setup surface functions to allow organisms to eat.
    surface.AddOverlapFun( [this](OpenOrg & pred, OpenOrg & prey) {
      const size_t pred_id = pred.GetSurfaceID();
      const size_t prey_id = prey.GetSurfaceID();
      const double pred_radius = surface.GetRadius(pred_id);
      const double prey_radius = surface.GetRadius(prey_id);
      const double consume_ratio = prey_radius / pred_radius;
      if (consume_ratio > config.MAX_CONSUME_RATIO()) return;
      if (consume_ratio < config.MIN_CONSUME_RATIO()) return;

      std::cerr << "Org Consumed!"
                << "   pred:radius=" << pred_radius << "; id=" << pred_id
                << "   prey:radius=" << prey_radius << "; id=" << prey_id
                << std::endl;
      pred.AdjustEnergy( prey_radius * prey_radius / 10.0 );
      // @CAO: MUST KILL PREY!!
    });
    surface.AddOverlapFun( [this](OpenOrg & org, OpenResource & res) {
      std::cerr << "Resoure Consumed!" << std::endl;
    });
    surface.AddOverlapFun( [](OpenResource &, OpenResource &) {
      std::cerr << "ERROR: Resources should not try to eat other resources!" << std::endl;
    });
    surface.AddOverlapFun( [](OpenResource &, OpenOrg &) {
      std::cerr << "ERROR: Resources should not try to eat organisms!" << std::endl;
    });

    // Setup the default instruction set.
    inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
    inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
    inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
    inst_lib.AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
    inst_lib.AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
    inst_lib.AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
    inst_lib.AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
    inst_lib.AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
    inst_lib.AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
    inst_lib.AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
    inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
    inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
    inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
    inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
    inst_lib.AddInst("CopyMem", hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
    inst_lib.AddInst("SwapMem", hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
    inst_lib.AddInst("Input", hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
    inst_lib.AddInst("Output", hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
    inst_lib.AddInst("Commit", hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
    inst_lib.AddInst("Pull", hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
    inst_lib.AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");
    inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
    inst_lib.AddInst("Terminate", hardware_t::Inst_Terminate, 0, "Kill current thread.");
    // These next five instructions are 'block'-modifying instructions: they facilitate within-function flow control. 
    // The {"block_def"} property tells the SignalGP virtual hardware that this instruction defines a new 'execution block'. 
    // The {"block_close"} property tells the SignalGP virtual hardware that this instruction exits the current 'execution block'. 
    inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
    inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");

    // Setup new instructions for the instruction set.
    inst_lib.AddInst("Vroom", [this](hardware_t & hw, const inst_t & inst) {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      emp::Angle facing = org_ptr->GetFacing();
      surface.TranslateWrap( org_ptr->GetSurfaceID(), facing.GetPoint(1.0) );
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

    inst_lib.AddInst("Consume", [this](hardware_t & hw, const inst_t & inst) mutable {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      surface.FindOverlap( org_ptr->GetSurfaceID() );  // Surface functions automatically try to eat on overlap!
    }, 1, "Rotate 5 degrees.");

    // On each update, run organisms and make sure they stay on the surface.
    OnUpdate([this](size_t){
      // Process all organisms.
      Process(5);

      // Update each organism.
      for (size_t pos = 0; pos < pop.size(); pos++) {
        if (pop[pos].IsNull()) continue;
        auto & org = *pop[pos];
        const size_t surface_id = org.GetSurfaceID();

        // Provide additional resources toward reproduction.
        org.AdjustEnergy( random_ptr->GetDouble(0.1) );

        // If an organism has enough energy to reproduce, do so.
        const size_t radius = surface.GetRadius(surface_id);
        const size_t mass = radius * radius;
        if (org.GetEnergy() > mass) {
          // Remove energy for building offspring; cut rest in half, so it is effectively
          // split between parent and child when copied into child.
          org.SetEnergy((org.GetEnergy() - mass / 2.0));
          DoBirth(org, pos);
//          emp::Alert("Birth!");
        }
      }
    });

    // Setup a mutation function.
    SetMutFun( [this](OpenOrg & org, emp::Random & random){
      signalgp_mutator.ApplyMutations(org.GetBrain().GetProgram(), random);
      double radius_change = emp::Pow2(random.GetDouble(-0.1, 0.1));
      surface.ScaleRadius(org.GetSurfaceID(), radius_change);
      return 1;
    });

    // Initialize a populaton of random organisms.
    Inject(OpenOrg(inst_lib, event_lib, random_ptr), config.INIT_POP_SIZE());
    for (size_t i = 0; i < config.INIT_POP_SIZE(); i++) {
      double x = random_ptr->GetDouble(config.WORLD_X());
      double y = random_ptr->GetDouble(config.WORLD_Y());
      OpenOrg & org = GetOrg(i);
      size_t surface_id = surface.AddBody(&org, {x,y}, 5.0);
      org.SetSurfaceID(surface_id);
      org.GetBrain().SetProgram(emp::GenRandSignalGPProgram(*random_ptr, inst_lib, config.PROGRAM_MIN_FUN_CNT(), config.PROGRAM_MAX_FUN_CNT(), config.PROGRAM_MIN_FUN_LEN(), config.PROGRAM_MAX_FUN_LEN(), config.PROGRAM_MIN_ARG_VAL(), config.PROGRAM_MAX_ARG_VAL()));
    }

    // Add in resources.
    resources.resize(config.NUM_RESOURCE_SOURCES());
    for (OpenResource & res : resources) {
      double x = random_ptr->GetDouble(config.WORLD_X());
      double y = random_ptr->GetDouble(config.WORLD_Y());
      res.surface_id = surface.AddBody(&res, {x,y}, 2.0);
    }
  }
  ~OpenWorld() { id_map.clear(); }

  surface_t & GetSurface() { return surface; }

  /// React to two bodies having collided.
  bool PairCollision(OpenOrg & body1, OpenOrg & body2) {
    return true;
  }
};

#endif
