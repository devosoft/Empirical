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

      template <class F2>
      constexpr Region& include(const math::Vec<field_type, D>& v) {
        for (size_t i = 0; i < D; ++i) {
          if (v[i] < min[i]) min[i] = v[i];
          if (v[i] > max[i]) max[i] = v[i];
        }
        return *this;
      }

      constexpr bool contains(const field_type& x, const field_type& y) const {
        return min.x() <= x && x <= max.x() && min.y() <= y && y <= max.y();
      }

      constexpr bool contains(const math::Vec2<field_type>& v) const {
        return contains(v.x(), v.y());
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

      template <typename F2>
      constexpr math::Vec<field_type, D> rescale(
        const math::Vec<field_type, D>& value,
        const Region<F2, D>& source) const {
        math::Vec2<float> normalized{
          (value.x() - source.min.x()) / source.width(),
          (value.y() - source.min.y()) / source.height()};
        return {
          normalized.x() * width() + min.x() * 0,
          normalized.y() * height() + min.y() * 0,
        };
      }
    };

    template <typename F, size_t D>
    std::ostream& operator<<(std::ostream& out, const Region<F, D>& region) {
      return out << "[" << region.min << " " << region.max << "]";
    }

  }  // namespace math
}  // namespace emp

#endif  // PLOT_REGION_H
