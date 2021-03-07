//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include "emp/geometry/Point2D.hpp"
#include "emp/geometry/Surface.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/tools/string_utils.hpp"

struct TestBody {
  size_t id = 0;
};

class TestSurface : public emp::Surface<TestBody> {
public:
  static constexpr double GetDim() { return 1000; }

  TestSurface() : emp::Surface<TestBody>({GetDim(),GetDim()}) { ; }

  std::string AsString(const TestBody & body) const {
    size_t x = (size_t) (GetCenter(body.id).GetX() + 0.5);
    size_t y = (size_t) (GetCenter(body.id).GetY() + 0.5);
    return emp::to_string(" (", x, ",", y, "):", (size_t) GetRadius(body.id));
  }
};


int main()
{
  constexpr size_t BODY_COUNT1 = 20;   // Starting body count
  constexpr size_t BODY_COUNT2 = 10;   // How far to cut down to?
  constexpr size_t BODY_COUNT3 = 15;    // How many new bodies to add later?

  TestSurface surface;

  emp::Random random;
  emp::vector<TestBody> bodies(BODY_COUNT1);

  for (auto & body : bodies) {
    body.id = surface.AddBody(&body,
			                        { random.GetDouble(surface.GetDim()), random.GetDouble(surface.GetDim()) },
		                  	      random.GetDouble(10.0, 100.0));
  }

  std::cout << "---- surface results ----" << std::endl;
  surface.AddOverlapFun(
    [&surface](TestBody & body1, TestBody & body2) {
      std::cout << "Overlap " << body1.id << " and " << body2.id
                << surface.AsString(body1) << " and " << surface.AsString(body2)
                << "  dist=" << surface.GetCenter(body1.id).Distance(surface.GetCenter(body2.id))
                << ".\n";
    }
  );
  surface.FindOverlaps();

//   std::cout << "\n---- brute force ----" << std::endl;
//   for (size_t id1 = 1; id1 < bodies.size(); id1++) {
//     auto & body1 = bodies[id1];
//     for (size_t id2 = 0; id2 < id1; id2++) {
//       auto & body2 = bodies[id2];
//       double radius_sum = body1.GetRadius() + body2.GetRadius();
//       if (body1.GetCenter().Distance(body2.GetCenter()) < radius_sum) {
//         PrintOverlap(body1, body2);
//       }
//     }
//   }

  std::cout << std::endl << "Moving bodies...\n";
  for (auto & body : bodies) {
    surface.SetCenter(body.id, { random.GetDouble(surface.GetDim()), random.GetDouble(surface.GetDim()) });
  }
  surface.FindOverlaps();

  std::cout << "\nREMOVING bodies " << BODY_COUNT2 << " and higher...\n";
  for (size_t i = BODY_COUNT2; i < BODY_COUNT1; i++) {
    surface.RemoveBody(bodies[i].id);
  }
  surface.FindOverlaps();


  std::cout << "\nAdding " << BODY_COUNT3 << " brand new bodies. ( ";
  emp::vector<TestBody> bodies3(BODY_COUNT3);

  for (auto & body : bodies3) {
    body.id = surface.AddBody(&body,
			                        { random.GetDouble(surface.GetDim()), random.GetDouble(surface.GetDim()) },
		                  	      random.GetDouble(10.0, 100.0));
    std::cout << body.id << " ";
  }
  std::cout << ")\n";
  surface.FindOverlaps();

}
