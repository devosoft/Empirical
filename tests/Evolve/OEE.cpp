/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file OEE.cpp
 */

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/Evolve/OEE.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/Evolve/World_output.hpp"

TEST_CASE("OEE", "[evo]") {
  // emp::Random random;
  // emp::World<int> world(random, "OEEWorld");

  // emp::Ptr<emp::Systematics<int, int> > sys_ptr;
  // sys_ptr.New([](int org){return org;}, true, true, false);

  // emp::OEETracker<emp::Systematics<int, int>> oee(sys_ptr, [](int org){return org;}, [](int org){return org;}, true);
  // oee.SetResolution(1);
  // oee.SetGenerationInterval(1);

  // sys_ptr->AddOrg(1, 0, 0, false);
  // sys_ptr->AddOrg(2, 1, 0, false);
  // sys_ptr->AddOrg(3, 2, 0, false);
  // sys_ptr->PrintStatus();
  // oee.Update(0);

  // // Coalescence interval hasn't passed yet
  // CHECK(oee.CoalescenceFilter(0).size() == 0);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 0);

  // sys_ptr->SetNextParent(0);
  // sys_ptr->RemoveOrgAfterRepro(2, 1);
  // sys_ptr->AddOrg(4, 2, 1, false);
  // sys_ptr->PrintStatus();
  // oee.Update(1);

  // // 1 and 2 should make it through filter
  // CHECK(oee.CoalescenceFilter(1).size() == 2);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 2);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 2);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 2);

  // // If we change nothing, 4 will now pass filter
  // oee.Update(2);
  // CHECK(oee.CoalescenceFilter(2).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

  // // If we change nothing again, change and novelty should drop to 0
  // oee.Update(3);
  // CHECK(oee.CoalescenceFilter(3).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

  // sys_ptr->SetNextParent(0);
  // sys_ptr->RemoveOrgAfterRepro(0,4);
  // sys_ptr->AddOrg(1, 0, 4, false);
  // sys_ptr->PrintStatus();

  // // Replacing 1 with a copy of itself should change nothing
  // oee.Update(4);
  // CHECK(oee.CoalescenceFilter(4).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

  // sys_ptr->SetNextParent(0);
  // sys_ptr->RemoveOrgAfterRepro(0, 5);
  // sys_ptr->AddOrg(10, 0, 5, false);
  // sys_ptr->PrintStatus();

  // // Replacing 1 with a new descendant should change nothing at first
  // // because 1 still has descendants and 10 hasn't survived filter time
  // oee.Update(5);
  // CHECK(oee.CoalescenceFilter(5).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

  // // 10 survives the filter and replaces 1 because 1 is no longer in the
  // // set being filtered
  // oee.Update(6);
  // CHECK(oee.CoalescenceFilter(6).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

  // sys_ptr->SetNextParent(0);
  // sys_ptr->RemoveOrgAfterRepro(1, 7);
  // sys_ptr->AddOrg(2, 1, 7, false);
  // sys_ptr->PrintStatus();

  // // Adding an independent origin of 2 should increase change but not novelty
  // // (the time after this). For now, we're replacing 2, leaving it with
  // // no descendants, so it should go away immediately
  // oee.Update(7);
  // CHECK(oee.CoalescenceFilter(7).size() == 2);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

  // // Now we see the bump in change
  // oee.Update(8);
  // CHECK(oee.CoalescenceFilter(8).size() == 3);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

  // sys_ptr->SetNextParent(0);
  // sys_ptr->AddOrg(10, 3, 9, false);
  // sys_ptr->PrintStatus();

  // // No effect this time
  // oee.Update(8);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

  // // Now we should see diversity change
  // oee.Update(9);
  // CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
  // CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5));
  // CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

  // sys_ptr.Delete();


}
