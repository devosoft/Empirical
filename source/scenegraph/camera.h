#ifndef EMP_OPENGL_CAMERA
#define EMP_OPENGL_CAMERA

#include <memory>
#include <vector>

#include "math/LinAlg.h"
#include "math/region.h"

namespace emp {
  namespace scenegraph {
    class Camera {
      public:
      virtual ~Camera() {}
      virtual math::Mat4x4f GetProjection() const = 0;
    };

    class OrthoCamera : public Camera {
      private:
      math::Region3f viewbox;

      public:
      OrthoCamera(const math::Region3f& viewbox) : viewbox(viewbox) {}

      math::Mat4x4f GetProjection() const override {
        return math::proj::ortho(viewbox.min, viewbox.max);
      }
    };

    class PerspectiveCamera : public Camera {
      private:
      float fov;
      float aspect;
      float near;
      float far;

      public:
      PerspectiveCamera(float fov, float aspect, float near, float far)
        : fov(fov), aspect(aspect), near(near), far(far) {}

      math::Mat4x4f GetProjection() const override {
        return math::proj::perspectiveFOV(fov, aspect, near, far);
      }
    };

    class Eye {
      public:
      virtual ~Eye() {}
      virtual math::Mat4x4f CalculateView() const = 0;
    };

    class SimpleEye : public Eye {
      math::Mat4x4f view;

      public:
      SimpleEye(const math::Mat4x4f& view = math::Mat4x4f::Identity())
        : view(view) {}

      void LookAt(const math::Vec3f& position, const math::Vec3f& target,
                  const math::Vec3f& up) {
        auto f{(target - position).Normalized()};
        auto UP = up.Normalized();
        auto s = Cross(UP, f);
        auto u = Cross(s.Normalized(), f);

        view = {s.x(),  s.y(),  s.z(),  0,  // row 1
                u.x(),  u.y(),  u.z(),  0,  // row 2
                -f.x(), -f.y(), -f.z(), 0,  // row 3
                0,      0,      0,      1};
        view = view * math::Mat4x4f::Translation(-position.x(), -position.y(),
                                                 -position.z());

        // auto zaxis = (position - target).Normalized();
        // auto xaxis = Cross(up, zaxis).Normalized();
        // auto yaxis = Cross(zaxis, xaxis).Normalized();
        // Mat4x4f orientation{xaxis.x(), yaxis.x(), zaxis.x(), 0};

        std::cout << view << std::endl;
      }

      math::Mat4x4f CalculateView() const override { return view; }
    };

  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_OPENGL_CAMERA
