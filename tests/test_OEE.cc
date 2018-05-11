#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../third-party/Catch/single_include/catch.hpp"

#include "Evolve/OEE.h"
#include "Evolve/World.h"
#include "Evolve/World_output.h"

TEST_CASE("OEE", "[evo]") {
    emp::Random random;
    emp::World<int> world(random, "OEEWorld");

    emp::Ptr<emp::Systematics<int, int> > sys_ptr;
    sys_ptr.New([](int org){return org;}, true, true, true);
    world.AddSystematics(sys_ptr);
    world.SetWellMixed(true);

    emp::OEETracker<int, int> oee(sys_ptr, [](int org){return org;});
    AddOEEFile(world, oee).SetTimingRepeat(10);
    world.OnUpdate([&oee](size_t ud){oee.Update(ud);});
    world.SetFitFun([](int & org){return org;});
    world.SetMutFun([](int & org, emp::Random r){
        if (r.P(.0025)) {
            org--;
        } else if (r.P(.0025)) {
            org++;
        } else {
            return 0;
        }
        return 1;
    });
    
    world.Inject(0);
    world.Inject(0);

    for (int i = 0; i < 10; i++) {
        EliteSelect(world, 1, 2);
        world.Update();
        oee.CalcStats();
    }

}