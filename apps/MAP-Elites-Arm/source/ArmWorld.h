#ifndef ARM_WORLD_H
#define ARM_WORLD_H

#include "base/vector.h"
#include "Evolve/World.h"
#include "geometry/Angle2D.h"
#include "geometry/Point2D.h"
#include "tools/Random.h"

struct ArmOrg {
  emp::vector<emp::Angle> angles;

  ArmOrg() : angles() { ; }
  ArmOrg(const ArmOrg &) = default;
  ArmOrg(ArmOrg &&) = default;
  ArmOrg(emp::Random & random, size_t in_size) {
    angles.resize(in_size);
    for (emp::Angle & angle : angles) angle.SetPortion(random.GetDouble());
  }

  double GetFitness() const {
    double sqr_diffs = 0.0;  // Total the squares of all of the differences between angles.
    for (size_t i = 1; i < angles.size(); i++) {
      double angle_diff = (angles[i] - angles[i-1]).AsPortion();
      if (angle_diff > 0.5) angle_diff = 1.0 - angle_diff;
      sqr_diffs += angle_diff * angle_diff;
    }
    return ((double) angles.size()) - sqr_diffs;
  }

  size_t DoMutations(emp::Random & random) {
    std::cout << "Ping!" << std::endl;
    if (random.P(0.5)) {
      size_t pos = random.GetUInt(angles.size());
      angles[pos].SetPortion(random.GetDouble());
      return 1;
    }
    return 0;
  }

  bool operator==(const ArmOrg & in) const { return angles == in.angles; }
  bool operator!=(const ArmOrg & in) const { return angles != in.angles; }
  bool operator< (const ArmOrg & in) const { return angles <  in.angles; }
  bool operator<=(const ArmOrg & in) const { return angles <= in.angles; }
  bool operator> (const ArmOrg & in) const { return angles >  in.angles; }
  bool operator>=(const ArmOrg & in) const { return angles >= in.angles; }

  std::string ToString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < angles.size(); i++) {
      if (i) ss << ',';
      ss << angles[i].AsPortion();
    }
    ss <<"]";
    return ss.str();
  }
};

class ArmWorld : public emp::World<ArmOrg> {
private:
  emp::vector<double> segments;
public:
  ArmWorld(emp::Random & random, emp::vector<double> in_segments={2.0,1.0,4.0,1.0,3.0})
    : emp::World<ArmOrg>(random, "ArmWorld"), segments(in_segments)
  {
    SetupFitnessFile().SetTimingRepeat(10);
    SetupSystematicsFile().SetTimingRepeat(10);
    SetupPopulationFile().SetTimingRepeat(10);
    SetWellMixed(true);
    SetCache();

    for (size_t i = 0; i < 100; i++) Inject(ArmOrg(random, segments.size()));
  }
  ~ArmWorld() { ; }

  emp::vector<emp::Point> CalcPoints(const ArmOrg & org) {
    emp::Angle facing(0);
    emp::Point position(0.0, 0.0);
    emp::vector<emp::Point> points(segments.size());
    for (size_t i = 0; i < segments.size(); i++) {
      facing += org.angles[i];
      position = facing.GetPoint(position, segments[i]);
      points[i] = position;
    }
    return points;
  }

  emp::Point CalcEndPoint(const ArmOrg & org) {
    emp::Angle facing(org.angles[0]);
    emp::Point position(facing.GetPoint(segments[0]));
    for (size_t i = 1; i < segments.size(); i++) {
      facing += org.angles[i];
      position = facing.GetPoint(position, segments[i]);
    }
    return position;
  }
};

#endif
