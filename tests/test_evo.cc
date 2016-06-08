#include <string>
#include <iostream>

#include "../evo/NK.h"
#include "../evo/World.h"
#include "../tools/Random.h"
#include "../evo/EvoStats.h"
#include "../evo/StatsManager.h"

using BitOrg = emp::BitVector;

template <typename ORG>
using MixedWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Base<ORG>>;


TEST_CASE("Test Stats ", "[evo]"){

    constexpr int N = 400;
    constexpr int UD_COUNT = 10;

    emp::Random random;
    MixedWorld<BitOrg> mixed_pop(random);

    std::function<double(BitOrg *)> fit_func = [](BitOrg * org) { return org->CountOnes(); };
    
    BitOrg next_orgA(N);
    for(int j = 0; j < N; j++) next_orgA[j] = 0;
    BitOrg next_orgB(N);
    for(int j = 0; j < N; j++) next_orgB[j] = 1;
    BitOrg next_orgC(N);
    for(int j = 0; j < N; j++){
        if(j % 2 == 0){next_orgC[j] = 0;}
        else{next_orgC[j] = 1;}
    }
    
    mixed_pop.Insert(next_orgA);
    mixed_pop.Insert(next_orgB);
    mixed_pop.Insert(next_orgC);

    double nf =  NonInf(fit_func, mixed_pop);
    REQUIRE(nf == 0.5);

}
