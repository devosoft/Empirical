#include "config/ArgManager.h"
#include "evo/NK.h"
#include "evo3/World.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

using BitOrg = emp::BitVector;

class NKWorld {
  emp::World<BitOrg> world;
};
