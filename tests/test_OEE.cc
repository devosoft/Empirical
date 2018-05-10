#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../third-party/Catch/single_include/catch.hpp"

#include "Evolve/OEE.h"
#include "Evolve/World.h"

TEST_CASE("OEE", "[evo]") {
    emp::Random random;
    emp::World<int> world(random, "OEEWorld");

    emp::Ptr<emp::Systematics<int, int> > sys_ptr;
    sys_ptr.New([](int org){return org;});
    world.AddSystematics(sys_ptr);
    world.SetWellMixed(true);

    emp::OEETracker<int, int> oee(sys_ptr);
    world.OnUpdate([&oee](size_t ud){oee.Update(ud);});
}