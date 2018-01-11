#ifndef PLOT_PROPERTIES_H
#define PLOT_PROPERTIES_H

#include <utility>
#include "static_type_map.h"

namespace emp {
  namespace plot {

    namespace properties {

      template <class N>
      struct Property {
        template <class M>
        static constexpr decltype(auto) get(const M& map) {
          static_assert(is_static_type_map_v<M>, "Get expects a StaticTypeMap");
          return emp::plot::get<N>(map);
        }

        template <class M, class V>
        static constexpr decltype(auto) set(M&& map, V&& value) {
          return concat(std::forward<M>(map),
                        Entry<N, std::decay_t<V>>{std::forward<V>(value)});
        }
      };

#define DEFINE_PROPERTY(NAME, _name)          \
  struct NAME : Property<NAME> {              \
    public:                                   \
    static constexpr auto name = #NAME;       \
  };                                          \
  template <class V>                          \
  constexpr Entry<NAME, V> _name(V&& value) { \
    return {std::forward<V>(value)};          \
  }
      template <class... Props>
      auto props(Props&&... props) {
        return staticTypeMap(std::forward<Props>(props)...);
      }

      template <class... E, class... Args>
      auto applyPropertyMap(const StaticTypeMap<E...>& props,
                            const Args&... args) {
        return staticTypeMap(entry<typename E::key_type>(
          get<typename E::key_type>(props)(args...))...);
      }

      DEFINE_PROPERTY(XY, xy);
      DEFINE_PROPERTY(XYScaled, xyScaled);
      DEFINE_PROPERTY(Stroke, stroke);
      DEFINE_PROPERTY(StrokeWeight, strokeWeight);
      DEFINE_PROPERTY(Fill, fill);

    }  // namespace properties
  }  // namespace plot
}  // namespace emp

#endif  // PLOT_PROPERTIES_H
