#ifndef ARM_WORLD_H
#define ARM_WORLD_H

#include "base/vector.h"
#include "Evolve/World.h"
#include "genometry/Angle.h"
#include "genometry/Point2D.h"
#include "tools/Random.h"

struct ArmOrg {
  emp::vector<emp::Angle> angles;
};

class ArmWorld : public emp::World<ArmOrg> {
private:
  emp::vector<double> segments;
  size_t num_angles;

  emp::Random random;
public:
  ArmWorld(emp::vector<double> in_segments)
    : segments(in_segments), num_angles(segments.size()) {
  }
  ~ArmWorld() { ; }

  void SetSegments(

  emp::Point CalcEndPoint(const ArmOrg & org) {
    emp::Angle facing(org.angles[0]);
    emp::Point position(facing.GetPoint(segments[0]));
    for (size_t i = 1; i < segments.size(); i++) {
      facing += org.angles[i];
      position = facing.GetPoint(position, segments[0]);
    }
    return position;
  }
};

#endif
