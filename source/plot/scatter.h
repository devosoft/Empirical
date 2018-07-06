#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include <cmath>
#include "flow.h"
#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "scenegraph/rendering.h"
#include "tools/attrs.h"

#include "tools/resources.h"

namespace emp {
  namespace plot {
    class Scatter : public scenegraph::Child {
      private:
      size_t vertexCount;
      std::vector<std::tuple<math::Mat4x4f, opengl::Color>> points;
      graphics::Mesh point_mesh;

      public:
      template <typename S = std::string>
      Scatter(emp::opengl::GLCanvas& canvas, graphics::Mesh point_mesh)
        : point_mesh(point_mesh) {
        using namespace emp::opengl;
        using namespace emp::math;
      }

      virtual ~Scatter() {}

      void RenderRelative(graphics::Graphics& g,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        using namespace emp::opengl;

        auto pen = g.Fill(point_mesh);

        for (auto& pt : points) {
          pen.Draw({
            emp::graphics::Fill = std::get<1>(pt),
            emp::graphics::Transform = std::get<0>(pt) * transform,
          });
        }

        pen.Flush();
      }

      template <typename DATA_ITER>
      void operator()(DATA_ITER begin, DATA_ITER end) {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        points.clear();

        for (; begin != end; ++begin) {
          auto model =
            Mat4x4f::Translation(graphics::Transform::Get(*begin).x(),
                                 graphics::Transform::Get(*begin).y(), 0) *
            Mat4x4f::Scale(PointSize::Get(*begin));

          points.push_back({model, graphics::Fill::Get(*begin)});
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
