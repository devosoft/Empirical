#ifndef STATIC_TYPE_MAP_H
#define STATIC_TYPE_MAP_H
#include <ostream>
#include "tuple_utils.h"

namespace emp {
  namespace plot {

    template <class... T>
    struct StaticTypeMap;

    template <class K, class V>
    struct Entry {
      using key_type = K;
      using value_type = V;
      V value;
    };
    template <class K, class V>
    std::ostream& operator<<(std::ostream& out, const Entry<K, V>& e) {
      return out << K::name << " = " << e.value;
    }

    template <class... Ks, class... Vs>
    struct StaticTypeMap<Entry<Ks, Vs>...> {
      private:
      using KeysType = std::tuple<Ks...>;
      using ValuesType = std::tuple<Entry<Ks, Vs>...>;

      public:
      ValuesType values;

      template <class Needle>
      static constexpr size_t indexOf = tuple_search<Needle, KeysType>::value;

      template <class Needle>
      static constexpr bool has = tuple_has<Needle, KeysType>::value;

      template <class Needle>
      using ValueType =
        typename std::tuple_element_t<indexOf<Needle>, ValuesType>::value_type;

      static constexpr size_t size = sizeof...(Vs);
    };

    template <class Needle, class Map>
    constexpr size_t IndexOf = Map::template indexOf<Needle>;

    template <class Needle, class Map>
    constexpr size_t Has = Map::template has<Needle>;

    template <class Needle, class Map>
    using GetValueType = typename Map::template ValueType<Needle>;

    template <class K, class V>
    Entry<K, V> entry(V&& value) {
      return {std::forward<V>(value)};
    }

    template <class... E>
    StaticTypeMap<std::decay_t<E>...> staticTypeMap(E&&... entries) {
      return {{std::forward<E>(entries)...}};
    }

    template <class... Ks1, class... Vs1, class... Ks2, class... Vs2>
    StaticTypeMap<Entry<Ks1, Vs1>..., Entry<Ks2, Vs2>...> concat(
      const StaticTypeMap<Entry<Ks1, Vs1>...>& map,
      const Entry<Ks2, Vs2>&... entries) {
      return {std::tuple_cat(map.values, std::make_tuple(entries...))};
    }

    template <class K1, class V1, class... E>
    StaticTypeMap<Entry<K1, V1>, E...> concat(const Entry<K1, V1>& first,
                                              E&&... entries) {
      return {{first, std::forward<E>(entries)...}};
    }

    template <class Needle, class... T>
    constexpr GetValueType<Needle, StaticTypeMap<T...>>& get(
      StaticTypeMap<T...>& map) noexcept {
      return std::get<IndexOf<Needle, StaticTypeMap<T...>>>(map.values).value;
    }

    template <class Needle, class... T>
    constexpr GetValueType<Needle, StaticTypeMap<T...>>&& get(
      StaticTypeMap<T...>&& map) noexcept {
      return std::move(
        std::get<IndexOf<Needle, StaticTypeMap<T...>>>(map.values).value);
    }

    template <class Needle, class... T>
    constexpr const GetValueType<Needle, StaticTypeMap<T...>>& get(
      const StaticTypeMap<T...>& map) noexcept {
      return std::get<IndexOf<Needle, StaticTypeMap<T...>>>(map.values).value;
    }

    template <class Needle, class... T>
    constexpr const GetValueType<Needle, StaticTypeMap<T...>>&& get(
      const StaticTypeMap<T...>&& map) noexcept {
      return std::move(
        std::get<IndexOf<Needle, StaticTypeMap<T...>>>(map.values).value);
    }
    namespace __impl {
      template <class T>
      struct is_static_type_map : std::false_type {};

      template <class... E>
      struct is_static_type_map<StaticTypeMap<E...>> : std::true_type {};
    }  // namespace __impl

    template <class T>
    using is_static_type_map = __impl::is_static_type_map<std::decay_t<T>>;
    template <class T>
    constexpr auto is_static_type_map_v = is_static_type_map<T>::value;

    namespace __impl {
      template <class...>
      struct pack {};

      template <class M>
      std::ostream& print(std::ostream& out, const M& map) {
        return out;
      }

      template <class M, class K>
      std::ostream& print(std::ostream& out, const M& map, const pack<K>&) {
        return out << K::name << " = " << get<K>(map);
      }

      template <class M, class K0, class K1, class... K>
      std::ostream& print(std::ostream& out, const M& map,
                          const pack<K0, K1, K...>&) {
        out << K0::name << ": " << get<K0>(map) << ", ";
        print(out, map, pack<K1, K...>{});
        return out;
      }
    }  // namespace __impl

    template <class... E>
    std::ostream& operator<<(std::ostream& out,
                             const StaticTypeMap<E...>& map) {
      out << "{ ";
      __impl::print(out, map, __impl::pack<typename E::key_type...>{});
      return out << " }";
    }
  }  // namespace plot
}  // namespace emp

#endif
