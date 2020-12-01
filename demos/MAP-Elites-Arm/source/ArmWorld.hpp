/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ArmWorld.hpp
 *  @brief Defines the specialized world for the MAP-Elites app.
 */

#ifndef ARM_WORLD_H
#define ARM_WORLD_H

#include "emp/base/vector.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/geometry/Angle2D.hpp"
#include "emp/geometry/Point2D.hpp"
#include "emp/math/Random.hpp"

struct ArmOrg {
  emp::vector<emp::Angle> angles;
  mutable emp::Point end_point;

  ArmOrg() : angles(), end_point(0.0, 0.0) { ; }
  ArmOrg(const ArmOrg &) = default;
  ArmOrg(ArmOrg &&) = default;
  ArmOrg(emp::Random & random, size_t in_size) : angles(), end_point(0.0,0.0) {
    angles.resize(in_size);
    for (emp::Angle & angle : angles) angle.SetPortion(random.GetDouble());
  }

  ArmOrg & operator=(const ArmOrg &) = default;
  ArmOrg & operator=(ArmOrg &&) = default;

  double GetFitness() const {
    double sqr_diffs = 0.0;  // Total the squares of all of the differences between angles.
    for (size_t i = 1; i < angles.size(); i++) {
      double angle_diff = (angles[i] - angles[i-1]).AsPortion();
      if (angle_diff > 0.5) angle_diff = 1.0 - angle_diff;
      sqr_diffs += angle_diff * angle_diff;
    }

    // Return a normalized fitness between 0.0 and 1.0.
    return (((double) angles.size()) - sqr_diffs * 4.0) / (double) angles.size();
  }

  size_t DoMutations(emp::Random & random) {
    if (random.P(1.0)) {
      size_t pos = random.GetUInt(angles.size());
      angles[pos].SetPortion(random.GetDouble());
      end_point.ToOrigin();
      return 1;
    }
    return 0;
  }

  emp::Point CalcEndPoint(const emp::vector<double> & segments) const {
    // If we haven't caclulated the end point yet, do so.
    if (end_point.AtOrigin()) {
      emp::Angle facing(angles[0]);
      end_point = facing.GetPoint(segments[0]);
      for (size_t i = 1; i < segments.size(); i++) {
        facing += angles[i];
        end_point = facing.GetPoint(end_point, segments[i]);
      }
    }

    return end_point;
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
protected:
  static constexpr size_t WORLD_X = 40;
  static constexpr size_t WORLD_Y = 40;
  static constexpr size_t WORLD_SIZE = WORLD_X * WORLD_Y;

  emp::vector<double> segments;
public:
  ArmWorld(emp::vector<double> in_segments={1,2,3,4,5,6})
    : emp::World<ArmOrg>("ArmWorld"), segments(in_segments)
  {
    NewRandom(1);

    SetupFitnessFile().SetTimingRepeat(10);
    SetupSystematicsFile().SetTimingRepeat(10);
    SetupPopulationFile().SetTimingRepeat(10);

    std::function<double(ArmOrg &)> traitX_fun = [this](ArmOrg & org){ return org.CalcEndPoint(segments).GetX(); };
    std::function<double(ArmOrg &)> traitY_fun = [this](ArmOrg & org){ return org.CalcEndPoint(segments).GetY(); };

    const double total = CalcTotalLength();

    AddPhenotype("End X", traitX_fun, -total, total);
    AddPhenotype("End Y", traitY_fun, -total, total);

    SetCache();
    SetAutoMutate();

    ResetMAP();
  }
  ~ArmWorld() { ; }

  void ResetMixed() {
    Reset();
    SetPopStruct_Mixed();
    for (size_t i = 0; i < WORLD_SIZE; i++) Inject(ArmOrg(*random_ptr, segments.size()));
  }

  void ResetMAP() {
    Reset();
    emp::SetMapElites(*this, {WORLD_X, WORLD_Y});
    for (size_t i = 0; i < 100; i++) Inject(ArmOrg(*random_ptr, segments.size()));
  }

  void ResetDiverse() {
    Reset();
    emp::SetDiverseElites(*this, WORLD_SIZE);
    for (size_t i = 0; i < WORLD_SIZE; i++) Inject(ArmOrg(*random_ptr, segments.size()));
  }

  double CalcTotalLength() const {
    double total = 0.0;
    for (auto x : segments) total += x;
    return total;
  }

  emp::vector<emp::Point> CalcPoints( const ArmOrg & org,
                                      emp::Point start_pos=emp::Point(0.0,0.0),
                                      double dilation=1.0 ) {
    emp::Angle facing(0);
    emp::Point position(start_pos);
    emp::vector<emp::Point> points(segments.size());
    for (size_t i = 0; i < segments.size(); i++) {
      facing += org.angles[i];
      position = facing.GetPoint(position, segments[i]*dilation);
      points[i] = position;
    }
    return points;
  }

  emp::Point CalcEndPoint(const ArmOrg & org) { return org.CalcEndPoint(segments); }
  emp::Point CalcEndPoint(size_t id) { return pop[id]->CalcEndPoint(segments); }

  emp::vector<emp::Point> CalcPoints( size_t id,
                                      emp::Point start_pos=emp::Point(0.0,0.0),
                                      double dilation=1.0 ) {
    return CalcPoints(*pop[id], start_pos, dilation);
  }

};

#endif
