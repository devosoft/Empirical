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
  OrthoCamera(const math::Region3f &viewbox) : viewbox(viewbox) {}

  void SetViewbox(const math::Region3f &viewbox) { this->viewbox = viewbox; }

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
  SimpleEye(const math::Mat4x4f &view = math::Mat4x4f::Identity())
      : view(view) {}

  void LookAt(const math::Vec3f &position, const math::Vec3f &target,
              const math::Vec3f &up) {
    auto f{(target - position).Normalized()};
    auto UP = up.Normalized();
    auto s = Cross(UP, f);
    auto u = Cross(s.Normalized(), f);

    view = {s.x(),  s.y(),  s.z(),  0, // row 1
            u.x(),  u.y(),  u.z(),  0, // row 2
            -f.x(), -f.y(), -f.z(), 0, // row 3
            0,      0,      0,      1};
    view = view * math::Mat4x4f::Translation(-position.x(), -position.y(),
                                             -position.z());

    // auto zaxis = (position - target).Normalized();
    // auto xaxis = Cross(up, zaxis).Normalized();
    // auto yaxis = Cross(zaxis, xaxis).Normalized();
    // Mat4x4f orientation{xaxis.x(), yaxis.x(), zaxis.x(), 0};
  }

  math::Mat4x4f CalculateView() const override { return view; }
};

class OrbitController {
  math::Vec3f position;
  float min_distance;
  float max_distance;
  float distance;
  math::Vec3f target;

public:
  template <typename T = math::Vec3f>
  OrbitController(const math::Vec3f &position, T &&target = {0, 0, 0})
      : OrbitController(position, (position - target).mag(),
                        std::forward<T>(target)) {}

  template <typename P = math::Vec3f, typename T = math::Vec3f>
  OrbitController(P &&position, float distance, T &&target = {0, 0, 0})
      : OrbitController(std::forward<P>(position), distance / 1.5,
                        distance * 1.5, distance, std::forward<T>(target)) {}

  template <typename P = math::Vec3f, typename T = math::Vec3f>
  OrbitController(P &&position, float min_distance, float max_distance,
                  T &&target = {0, 0, 0})
      : OrbitController(std::forward<P>(position), min_distance, max_distance,
                        (max_distance - min_distance) / 2,
                        std::forward<T>(target)) {}

  template <typename P = math::Vec3f, typename T = math::Vec3f>
  OrbitController(P &&position, float min_distance, float max_distance,
                  float distance, T &&target = {0, 0, 0})
      : position(std::forward<P>(position)), min_distance(min_distance),
        max_distance(max_distance), distance(distance),
        target(std::forward<T>(target)) {}

  void Move(const math::Vec3f &delta) {
    position = (position + delta - target).Normalized() * distance + target;
  }

  void Zoom(float delta) {
    distance += delta;
    if (distance < min_distance)
      distance = min_distance;
    if (distance > max_distance)
      distance = max_distance;
  }

  void Apply(SimpleEye &eye, const math::Vec3f &up = {0, 0, -1}) const {
    eye.LookAt(position, target, up);
  }
};

} // namespace scenegraph
} // namespace emp

#endif // EMP_OPENGL_CAMERA
