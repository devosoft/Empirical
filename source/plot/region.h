#ifndef PLOT_REGION_H
#define PLOT_REGION_H
namespace emp {
  namespace plot {

    template <typename F>
    class Region2D {
      public:
      Vec2<F> min;
      Vec2<F> max;

      constexpr Region2D(F minX, F minY, F maxX, F maxY)
        : min{minX, minY}, max{maxX, maxY} {}
      constexpr Region2D(const Vec2<F>& min, const Vec2<F>& max)
        : min{min}, max{max} {}

      constexpr Region2D(const Region2D&) = default;
      constexpr Region2D(Region2D&&) = default;

      constexpr Region2D& operator=(const Region2D&) = default;
      constexpr Region2D& operator=(Region2D&&) = default;

      constexpr decltype(auto) width() const { return max.x - min.x; }
      constexpr decltype(auto) height() const { return max.y - min.y; }
      constexpr decltype(auto) size() const { return max - min; }

      Vec2<F> rescale(const Vec2<F>& value, const Region2D<F>& from) const {
        return {
          ((value.x - from.min.x) / from.width()) * width() + min.x,
          ((value.y - from.min.y) / from.height()) * height() + min.y,
        };
      }
    };

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_REGION_H
