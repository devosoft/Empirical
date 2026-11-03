#include <set>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"
#include "emp/geometry/Physics2D.hpp"

namespace {
template <typename T> std::string State(T & physics) {
  std::ostringstream output;
  emp::SerialPod pod{output};
  pod(physics);
  return output.str();
}
template <typename T> void Restore(T & physics, const std::string & state) {
  std::istringstream input{state};
  emp::SerialPod pod{input};
  pod(physics);
  REQUIRE_FALSE(input.fail());
  REQUIRE(physics.LoadedStateOK());
}
struct PayloadBody : emp::PhysicsBody {
  size_t organism = emp::MAX_SIZE_T;
  using emp::PhysicsBody::PhysicsBody;
  void Serialize(emp::SerialPod & pod) { emp::PhysicsBody::Serialize(pod); pod(organism); }
};
}

TEST_CASE("Physics collisions have finite shifts and conserve momentum", "[geometry]") {
  emp::Physics physics{emp::Size2D{512, 512}};
  const auto a = physics.AddBody(emp::Circle{emp::Point{10, 10}, 1});
  const auto b = physics.AddBody(emp::Circle{emp::Point{11, 11}, 1});
  physics.GetBody(a).SetVelocity({1, 1});
  REQUIRE(physics.ProcessPairCollision(physics.GetBody(a), physics.GetBody(b)));
  const auto momentum = physics.GetVelocity(a) + physics.GetVelocity(b);
  REQUIRE(momentum.X() == Approx(1));
  REQUIRE(momentum.Y() == Approx(1));
  physics.MoveTo(b, {10, 10});
  REQUIRE(physics.ProcessPairCollision(physics.GetBody(a), physics.GetBody(b)));
  REQUIRE(std::isfinite(physics.GetBody(a).CalcShiftPos().X()));
  REQUIRE(std::isfinite(physics.GetBody(b).CalcShiftPos().Y()));
}

TEST_CASE("Toroidal physics wraps shifts without artificial pressure", "[geometry]") {
  emp::Physics physics{emp::Size2D{512, 512}};
  physics.SetWrap(true);
  physics.SetPressureLimit(0);
  const auto a = physics.AddBody(emp::Circle{emp::Point{511, 1}, 1});
  physics.GetBody(a).SetVelocity({1026, -1026});
  physics.Update();
  REQUIRE(physics.IsActive(a));
  REQUIRE(physics.GetBody(a).X() == Approx(1));
  REQUIRE(physics.GetBody(a).Y() == Approx(511));
  REQUIRE(physics.LoadedStateOK());
  physics.MoveTo(a, {0.5, 0.5});
  const auto b = physics.AddBody(emp::Circle{emp::Point{511.5, 511.5}, 1});
  size_t pairs = 0;
  physics.TriggerOverlaps([&](size_t, size_t) { ++pairs; });
  REQUIRE(pairs == 1);
  REQUIRE(physics.TestOverlap(a, b));
  REQUIRE(physics.ProcessPairCollision(physics.GetBody(a), physics.GetBody(b)));
}

TEST_CASE("Wrapped sector neighbors do not duplicate collisions", "[geometry]") {
  for (const auto size : {emp::Size2D{10, 10}, emp::Size2D{10, 30}, emp::Size2D{30, 10}}) {
    emp::Physics physics{size};
    physics.SetWrap(true);
    physics.AddBody(emp::Circle{emp::Point{0.1, 0.1}, 3});
    physics.AddBody(emp::Circle{emp::Point{size.Width() - 0.1, size.Height() - 0.1}, 3});
    size_t pairs = 0;
    physics.TriggerOverlaps([&](size_t, size_t) { ++pairs; });
    REQUIRE(pairs == 1);
  }
}

TEST_CASE("Growth updates the collision index", "[geometry]") {
  emp::Physics physics{emp::Size2D{512, 512}};
  const auto a = physics.AddBody(emp::Circle{emp::Point{15.9, 100}, 2});
  const auto b = physics.AddBody(emp::Circle{emp::Point{48.1, 100}, 2});
  physics.GetBody(a).SetTargetRadius(20);
  physics.GetBody(b).SetTargetRadius(20);
  for (size_t step = 0; step < 400; ++step) physics.Update_Bodies();
  REQUIRE(physics.TestOverlap(a, b));
  size_t pairs = 0;
  physics.TriggerOverlaps([&](size_t, size_t) { ++pairs; });
  REQUIRE(pairs == 1);
}

TEST_CASE("Pressure death uses the configured limit", "[geometry]") {
  emp::Physics physics{emp::Size2D{512, 512}};
  const auto a = physics.AddBody(emp::Circle{emp::Point{100, 100}, 3});
  physics.SetPressureLimit(3);
  physics.GetBody(a).ProcessShift({1, 0});
  physics.GetBody(a).ProcessShift({-1, 0});
  physics.Update_FinalizePositions();
  REQUIRE_FALSE(physics.IsActive(a));
  const auto b = physics.AddBody(emp::Circle{emp::Point{100, 100}, 3});
  REQUIRE(b == a);
  REQUIRE(physics.GetBody(b).CalcPressure() == 0);
  physics.Update();
  REQUIRE(physics.IsActive(b));
}

TEST_CASE("Physics checkpoints preserve payloads, holes, ordering and exact continuation", "[geometry]") {
  using Physics = emp::Physics2D<PayloadBody>;
  Physics physics{emp::Size2D{512, 512}};
  physics.SetWrap(true);
  physics.SetPressureLimit(100);
  for (size_t id = 0; id < 20; ++id) {
    const auto body = physics.AddBody(emp::Circle{emp::Point{100.25 + id, 100.75}, 2});
    physics.GetBody(body).organism = 100 + id;
    physics.GetBody(body).SetVelocity({0.12345678901234567, -0.05});
    physics.GetBody(body).SetTargetRadius(3);
    physics.GetBody(body).RotateDegrees(123.456);
  }
  physics.RemoveBody(4);
  physics.RemoveBody(9);
  // Restore with pending shifts as well as settled body positions.
  physics.GetBody(0).ProcessShift({0.3141592653589793, 0.2718281828459045});
  Physics restored{emp::Size2D{100, 100}};
  Restore(restored, State(physics));
  REQUIRE(State(restored) == State(physics));
  REQUIRE(restored.GetBody(0).organism == 100);
  const emp::Circle circle{emp::Point{110, 110}, 2};
  REQUIRE(physics.AddBody(circle) == restored.AddBody(circle));
  for (size_t step = 0; step < 50; ++step) {
    physics.Update();
    restored.Update();
    REQUIRE(State(physics) == State(restored));
    REQUIRE(restored.LoadedStateOK());
  }
  physics.Clear();
  Restore(restored, State(physics));
  REQUIRE(State(physics) == State(restored));
}
