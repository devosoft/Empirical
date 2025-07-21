/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2025
*/
/**
 *  @file
 */

#include "emp/geometry/Physics2D.hpp"
#include "emp/geometry/Surface.hpp"
#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/CanvasShape.hpp"
#include "emp/web/Color.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

struct TestBody : public emp::PhysicsBody {
  TestBody(emp::Circle circle = emp::Circle{1.0}, emp::Color color = emp::Palette::BLACK)
    : PhysicsBody(circle, color) { }
};

const emp::Size2D can_size{800,800};
// emp::Surface<TestBody> surface(can_size);
emp::Physics2D<TestBody> physics(can_size);

int emp_main()
{
  DEBUG_STACK();
  emp::Random random;

  UI::Document doc{"emp_base"};

  emp::vector<size_t> body_ids;

  body_ids.push_back( physics.AddBody(emp::Circle{{150,150}, 50}) );
  body_ids.push_back( physics.AddBody(emp::Circle{{215,160}, 50}) );
  body_ids.push_back( physics.AddBody(emp::Circle{{650,150}, 50}) );
  body_ids.push_back( physics.AddBody(emp::Circle{{770,360}, 50}) ); // Off right
  body_ids.push_back( physics.AddBody(emp::Circle{{200,785}, 50}) ); // Off bottom
  body_ids.push_back( physics.AddBody(emp::Circle{{20,500}, 50}) ); // Off left
  body_ids.push_back( physics.AddBody(emp::Circle{{550,45}, 50}) ); // Off top
  body_ids.push_back( physics.AddBody(emp::Circle{{10,15}, 50}) ); // Off UL
  body_ids.push_back( physics.AddBody(emp::Circle{{775,775}, 50}) ); // Off BR


  // auto & body1 = physics.GetBody(id1);
  // auto & body2 = physics.GetBody(id2);
  // auto & body3 = physics.GetBody(id3);
  // auto & body4 = physics.GetBody(id4);

  auto canvas = doc.AddCanvas(can_size, "can");

  // Draw the circles.
  canvas.Clear();
  UI::Draw(canvas, physics);

  physics.Update_Bodies();
  physics.Update_BodyCollisions();
  physics.Update_EdgeCollisions();

  for (size_t body_id : body_ids) {
    auto & body = physics.GetBody(body_id);
    // Draw trajectory.
    canvas.Draw(body.GetCenter(), body.CalcShiftPos(), emp::Palette::MAGENTA);

    // Draw body Center
    canvas.Draw(emp::Circle{body.GetCenter(), 3}, emp::Palette::NONE, emp::Palette::YELLOW);

    // Draw shift result
    canvas.Draw(emp::Circle{body.CalcShiftPos(), 3}, emp::Palette::NONE, emp::Palette::ORANGE);
  }

  // // Draw a point directly between the centers of the two circles.
  // canvas.Draw(emp::Circle{body1.GetCenter().Midpoint(body2.GetCenter()), 3},
  //             emp::Palette::NONE, emp::Palette::CYAN);
}
