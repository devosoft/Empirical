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
    // world.AddSystematics(sys_ptr);
    // world.SetPopStruct_Mixed(true);

    emp::OEETracker<int, int> oee(sys_ptr, [](emp::Ptr<emp::Taxon<int>> org){std::cout << "In complexity fun " << org << std::endl; return org->GetInfo();});
    oee.SetResolution(1);
    oee.SetGenerationInterval(1);
    // AddOEEFile(world, oee).SetTimingRepeat(10);
    // world.OnUpdate([&oee](size_t ud){oee.Update(ud);});
    // world.SetFitFun([](int & org){return org;});
    // world.SetMutFun([](int & org, emp::Random r){
    //     if (r.P(.0025)) {
    //         org--;
    //     } else if (r.P(.0025)) {
    //         org++;
    //     } else {
    //         return 0;
    //     }
    //     return 1;
    // });        
    
    sys_ptr->AddOrg(1, 0, 0, false);
    sys_ptr->AddOrg(2, 1, 0, false);
    sys_ptr->AddOrg(3, 2, 0, false);
    sys_ptr->PrintStatus();
    oee.Update(0);
    
    // Coalescence interval hasn't passed yet
    CHECK(oee.CoalescenceFilter().size() == 0);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 0);

    sys_ptr->SetNextParent(0);
    sys_ptr->RemoveOrgAfterRepro(2);
    sys_ptr->AddOrg(4, 2, 0, false);
    sys_ptr->PrintStatus();
    oee.Update(1);
    
    // 1 and 2 should make it through filter
    CHECK(oee.CoalescenceFilter().size() == 2);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 2);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 2);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 2);

    // If we change nothing, 4 will now pass filter
    oee.Update(2);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.58496));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);


    sys_ptr.Delete();


}