#ifndef EMP_OPENGL_CAMERA
#define EMP_OPENGL_CAMERA

#include "math/LinAlg.h"
#include "math/region.h"

namespace emp {
  namespace scenegraph {
    class Camera {
      public:
      virtual ~Camera() {}
      virtual math::Mat4x4f getProjection() const = 0;
      virtual math::Mat4x4f getView() const = 0;
      virtual math::Region2f getRegion() const = 0;
    };

    class OrthoCamera : public Camera {
      private:
      math::Region2f region;
      math::Mat4x4f projection;
      math::Mat4x4f view;

      void calculateMatrices() {
        projection = math::proj::orthoFromScreen(
          region.extents().x(), region.extents().y(), region.extents().x(),
          region.extents().y());
        view = math::Mat4x4f::translation(0, 0, 0);
      }

      public:
      ~OrthoCamera() override {}
      template <class R = decltype(region)>
      OrthoCamera(R&& region)
        : region(std::forward<R>(region)),
          projection(math::proj::orthoFromScreen(
            region.width(), region.height(), region.width(), region.height())),
          view(math::Mat4x4f::translation(0, 0, 0)) {}

      template <class R = decltype(region)>
      void setRegion(R&& region) {
        this->region = std::forward<R>(region);
        calculateMatrices();
      }

      math::Region2f getRegion() const override { return region; }

      math::Mat4x4f getProjection() const override { return projection; }
      math::Mat4x4f getView() const override { return view; }
    };

    class PerspectiveCamera : public Camera {
      private:
      math::Region2f region;
      math::Mat4x4f projection;
      math::Mat4x4f view;

      void calculateMatrices() {
        projection = math::proj::perspective(
          -region.extents().x() / 2, -region.extents().x() / 2,
          region.extents().y() / 2, region.extents().y() / 2);
        // projection = math::proj::orthoFromScreen(
        //   region.width(), region.height(), region.width(), region.height());
        view = math::Mat4x4f::translation(0, 0, 0);
      }

      public:
      ~PerspectiveCamera() override {}
      template <class R = decltype(region)>
      PerspectiveCamera(R&& region)
        : region(std::forward<R>(region)),
          projection(math::proj::orthoFromScreen(
            region.extents().x(), region.extents().y(), region.extents().x(),
            region.extents().y())),
          view(math::Mat4x4f::translation(0, 0, 0)) {}

      template <class R = decltype(region)>
      void setRegion(R&& region) {
        this->region = std::forward<R>(region);
        calculateMatrices();
      }

      math::Region2f getRegion() const override { return region; }

      math::Mat4x4f getProjection() const override { return projection; }
      math::Mat4x4f getView() const override { return view; }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_OPENGL_CAMERA
