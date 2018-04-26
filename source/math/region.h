#ifndef PLOT_REGION_H
#define PLOT_REGION_H
#include <limits>
#include <type_traits>
#include "math/LinAlg.h"

namespace emp {
  namespace math {

    template <typename F, size_t D>
    class Region {
      using field_type = std::decay_t<F>;

      public:
      math::Vec<field_type, D> min;
      math::Vec<field_type, D> max;

      constexpr Region(const math::Vec<field_type, D>& min =
                         {std::numeric_limits<field_type>::max()},
                       const math::Vec<field_type, D>& max =
                         {std::numeric_limits<field_type>::lowest()})
        : min{min}, max{max} {}

      constexpr Region(const Region&) = default;
      constexpr Region(Region&&) = default;

      constexpr Region& operator=(const Region&) = default;
      constexpr Region& operator=(Region&&) = default;

      constexpr auto extents() const {
        auto size = max - min;
        for (size_t i = 0; i < D; ++i) {
          if (size[i] < 0) size[i] = 0;
        }
        return size;
      }

      constexpr auto Center() const { return (max - min) / 2; }

      template <class F2 = field_type>
      constexpr Region& include(const math::Vec<F2, D>& v) {
        for (size_t i = 0; i < D; ++i) {
          if (v[i] < min[i]) min[i] = v[i];
          if (v[i] > max[i]) max[i] = v[i];
        }
        return *this;
      }

      constexpr bool contains(const math::Vec2<field_type>& v) const {
        return min <= v && max <= v;
      }

      template <class F2>
      constexpr Region& addBorder(const math::Vec<F2, D>& border) {
        min -= border;
        max += border;
        return *this;
      }

      constexpr Region& addBorder(const field_type& border) {
        return addBorder({border});
      }

      template <typename F2 = field_type, typename F3 = field_type>
      constexpr math::Vec<field_type, D> RescalePoint(
        const math::Vec<F2, D>& point, const Region<F3, D>& point_space) const {
        using namespace emp::math;
        return mult(div(point - point_space.min, point_space.extents()),
                    extents()) +
               min;
      }

      template <typename S1 = field_type, typename S2 = field_type>
      constexpr Region& Scale(const math::Vec<S1, D>& scale,
                              const math::Vec<S2, D>& origin) {
        min = mult(min - origin, scale) + origin;
        max = mult(max - origin, scale) + origin;

        return *this;
      }

      template <typename S = field_type>
      constexpr Region& Scale(const math::Vec<S, D>& scale) {
        return Scale(scale, Center());
      }
      template <typename S = field_type>
      constexpr Region Scaled(const math::Vec<S, D>& scale) const {
        auto scaled = *this;
        scaled.Scale(scale);
        return scaled;
      }
      template <typename S1 = field_type, typename S2 = field_type>
      constexpr Region Scaled(const math::Vec<S1, D>& scale,
                              const math::Vec<S2, D>& origin) const {
        auto scaled = *this;
        scaled.Scale(scale, origin);
        return scaled;
      }

      template <typename S = field_type>
      constexpr Region& Translate(const math::Vec<S, D>& delta) {
        min += delta;
        max += delta;
        return *this;
      }
      template <typename S = field_type>
      constexpr Region Translated(const math::Vec<S, D>& delta) const {
        auto translated = *this;
        translated.Translate(delta);
        return translated;
      }

      template <typename... U>
      constexpr Region<F, sizeof...(U)> Only(U&&... args) const {
        return {min.Only(args...), max.Only(std::forward<U>(args)...)};
      }

      constexpr Region<F, D - 1> DropDimension() const {
        return {min.DropDimension(), max.DropDimension()};
      }

      template <typename F1 = F, typename F2 = F>
      constexpr Region<F, D + 1> AddDimension(F1&& min, F2&& max) const {
        return {this->min.AddRow(std::forward<F1>(min)),
                this->max.AddRow(std::forward<F2>(max))};
      }
    };  // namespace math

    template <typename F, size_t D>
    std::ostream& operator<<(std::ostream& out, const Region<F, D>& region) {
      return out << "[" << region.min << " " << region.max << "]";
    }

    using Region2i = Region<int, 2>;
    using Region2f = Region<float, 2>;
    using Region2d = Region<double, 2>;
    using Region3i = Region<int, 3>;
    using Region3f = Region<float, 3>;
    using Region3d = Region<double, 3>;

    template <typename F>
    constexpr auto AspectRatio(const Region<F, 2>& region) {
      auto extents = region.extents();
      return extents.x() / extents.y();
    }

    template <typename F, typename... U>
    constexpr Region<F, 2> SetAspectRatioWidth(const Region<F, 2>& region,
                                               float aspect, U&&... args) {
      auto extents = region.extents();
      return region.Scaled({1, extents.x() / (aspect * extents.y())},
                           std::forward<U>(args)...);
    }

    template <typename F, typename... U>
    constexpr Region<F, 2> SetAspectRatioHeight(const Region<F, 2>& region,
                                                float aspect, U&&... args) {
      auto extents = region.extents();
      return region.Scaled({aspect * (extents.y() / extents.x()), 1},
                           std::forward<U>(args)...);
    }

    template <typename F, typename... U>
    constexpr Region<F, 2> SetAspectRatioMax(const Region<F, 2>& region,
                                             float aspect, U&&... args) {
      auto extents = region.extents();
      if (extents.x() > extents.y()) {
        return SetAspectRatioWidth(region, aspect, std::forward<U>(args)...);
      } else {
        return SetAspectRatioHeight(region, aspect, std::forward<U>(args)...);
      }
    }

    template <typename F, typename... U>
    constexpr Region<F, 2> SetAspectRatioMin(const Region<F, 2>& region,
                                             float aspect, U&&... args) {
      auto extents = region.extents();
      if (extents.x() < extents.y()) {
        return SetAspectRatioHeight(region, aspect, std::forward<U>(args)...);
      } else {
        return SetAspectRatioWidth(region, aspect, std::forward<U>(args)...);
      }
    }

  }  // namespace math
}  // namespace emp

#endif  // PLOT_REGION_H
