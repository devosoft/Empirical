#ifndef PLOT_REGION_H
#define PLOT_REGION_H
#include <limits>

namespace emp {
  namespace math {

    template <typename F>
    class Region2D {
      using field_type = std::decay_t<F>;

      public:
      math::Vec2<field_type> min;
      math::Vec2<field_type> max;

      constexpr Region2D(
        field_type minX = std::numeric_limits<field_type>::max(),
        field_type minY = std::numeric_limits<field_type>::max(),
        field_type maxX = std::numeric_limits<field_type>::lowest(),
        field_type maxY = std::numeric_limits<field_type>::lowest())
        : min{minX, minY}, max{maxX, maxY} {}

      constexpr Region2D(const math::Vec2<field_type>& min,
                         const math::Vec2<field_type>& max)
        : min{min}, max{max} {}

      constexpr Region2D(const Region2D&) = default;
      constexpr Region2D(Region2D&&) = default;

      constexpr Region2D& operator=(const Region2D&) = default;
      constexpr Region2D& operator=(Region2D&&) = default;

      constexpr auto width() const { return size().x(); }
      constexpr auto height() const { return size().y(); }
      constexpr auto size() const {
        if (max.x() >= min.x() && max.y() >= min.y()) {
          return max - min;
        }
        return Vec2<field_type>{0, 0};
      }

      constexpr Region2D& include(const field_type& x, const field_type& y) {
        if (x < min.x()) min.x() = x;
        if (x > max.x()) max.x() = x;

        if (y < min.y()) min.y() = y;
        if (y > max.y()) max.y() = y;

        return *this;
      }

      constexpr Region2D& include(const math::Vec2<field_type>& v) {
        return include(v.x(), v.y());
      }

      constexpr bool contains(const field_type& x, const field_type& y) const {
        return min.x() <= x && x <= max.x() && min.y() <= y && y <= max.y();
      }

      constexpr bool contains(const math::Vec2<field_type>& v) const {
        return contains(v.x(), v.y());
      }

      constexpr Region2D& addBorder(const math::Vec2<field_type>& border) {
        min -= border;
        max += border;
        return *this;
      }

      constexpr Region2D& addBorder(const field_type& dx,
                                    const field_type& dy) {
        return addBorder({dx, dy});
      }

      constexpr Region2D& addBorder(const field_type& border) {
        return addBorder(border, border);
      }

      template <typename F2>
      constexpr math::Vec2<field_type> rescale(
        const math::Vec2<field_type>& value, const Region2D<F2>& from) const {
        math::Vec2<float> normalized{
          (value.x() - from.min.x()) / from.width(),
          (value.y() - from.min.y()) / from.height()};
        return {
          normalized.x() * width() + min.x() * 0,
          normalized.y() * height() + min.y() * 0,
        };
      }
    };

    template <typename F>
    std::ostream& operator<<(std::ostream& out, const Region2D<F>& region) {
      return out << "[" << region.min << " " << region.max << "]";
    }

  }  // namespace math
}  // namespace emp

#endif  // PLOT_REGION_H
