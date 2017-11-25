#ifndef PLOT_PROPERTIES_H
#define PLOT_PROPERTIES_H

namespace emp {
  namespace plot {

    namespace properties {

      namespace detail {
        template <typename P, typename V>
        struct PropertySetter {
          V value;

          template <typename Props>
          constexpr decltype(auto) operator()(Props&& props) const {
            return std::forward<Props>(props).template set<P>(value);
          }
        };

        template <typename Props, typename P, typename V>
        constexpr decltype(auto) operator>>(
          Props&& props, const PropertySetter<P, V>& setter) {
          return setter(std::forward<Props>(props));
        }
      }  // namespace detail

      template <typename P>
      struct PropertyName {
        template <typename V>
        static constexpr auto is(V&& map) {
          return detail::PropertySetter<P, V>{std::forward<V>(map)};
        }

        template <typename Props>
        static constexpr decltype(auto) get(Props&& properties) {
          return std::forward<Props>(properties).template get<P>();
        }
      };

      struct Fill : PropertyName<Fill> {};
      struct Stroke : PropertyName<Stroke> {};
      struct FillShader : PropertyName<FillShader> {};
      struct StrokeShader : PropertyName<StrokeShader> {};
      struct StrokeWeight : PropertyName<StrokeWeight> {};

      struct X : PropertyName<X> {};
      struct Y : PropertyName<Y> {};

      struct ScaledX : PropertyName<ScaledX> {};
      struct ScaledY : PropertyName<ScaledY> {};

      struct Value : PropertyName<Value> {};
    };  // namespace properties

    namespace detail {
      template <typename U, typename T>
      struct TupleHas : std::false_type {};

      template <typename U, typename... T>
      struct TupleHas<U, std::tuple<U, T...>> : std::true_type {};

      template <typename U, typename H, typename... T>
      struct TupleHas<U, std::tuple<H, T...>> : TupleHas<U, std::tuple<T...>> {
      };

      template <size_t I, typename U, typename T>
      struct IndexOfImpl;

      template <size_t I, typename U, typename... T>
      struct IndexOfImpl<I, U, std::tuple<U, T...>>
        : std::integral_constant<size_t, I> {};

      template <size_t I, typename U, typename H, typename... T>
      struct IndexOfImpl<I, U, std::tuple<H, T...>>
        : IndexOfImpl<I + 1, U, std::tuple<T...>> {};

      template <typename U, typename T>
      struct IndexOf : IndexOfImpl<0, U, T> {};

    }  // namespace detail

    template <typename K, typename P>
    class Props;

    template <typename... K, typename... P>
    class Props<std::tuple<K...>, std::tuple<P...>> {
      private:
      using keys_type = std::tuple<K...>;
      using properties_type = std::tuple<P...>;

      using decayed_keys_type = std::tuple<std::decay_t<K>...>;
      using decayed_properties_type = std::tuple<std::decay_t<P>...>;

      properties_type properties;

      public:
      Props() = default;
      Props(const Props&) = default;
      Props(Props&&) = default;

      Props(const properties_type& properties) : properties(properties) {}
      Props(properties_type&& properties) : properties(std::move(properties)) {}

      Props& operator=(const Props&) = default;
      Props& operator=(Props&&) = default;

      template <typename U>
      static constexpr bool has() {
        return detail::TupleHas<std::decay_t<U>, decayed_keys_type>::value;
      }

      template <typename U>
      constexpr decltype(auto) get() const {
        static_assert(Props::has<U>(), "No such property");
        constexpr auto index =
          detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
        return std::get<index>(properties);
      }

      template <typename U>
      constexpr decltype(auto) get() {
        static_assert(Props::has<U>(), "No such property");
        constexpr auto index =
          detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
        return std::get<index>(properties);
      }

      template <typename U, typename D>
      constexpr decltype(auto) get(
        D&& defaultCallback, std::enable_if<Props::has<U>(), char> = 0) const {
        return get<U>();
      }

      template <typename U, typename D>
      constexpr decltype(auto) get(
        D&& defaultCallback, std::enable_if<!Props::has<U>(), char> = 0) const {
        return std::forward<D>(defaultCallback)();
      }

      template <typename U, typename D>
      constexpr decltype(auto) get(D&& defaultCallback,
                                   std::enable_if<Props::has<U>(), char> = 0) {
        return get<U>();
      }

      template <typename U, typename D>
      constexpr decltype(auto) get(D&& defaultCallback,
                                   std::enable_if<!Props::has<U>(), char> = 0) {
        return std::forward<D>(defaultCallback)();
      }

      template <typename U>
      constexpr auto set(U&& value) -> std::enable_if<Props::has<U>(), Props&> {
        get<U>() = std::forward<U>(value);
        return *this;
      }

      template <typename Key, typename Value>
      constexpr auto set(Value value) -> typename std::enable_if<
        !Props::has<Key>(),
        Props<std::tuple<Key, K...>, std::tuple<Value, P...>>>::type {
        return Props<std::tuple<Key, K...>, std::tuple<Value, P...>>{
          std::tuple_cat(std::make_tuple(value), properties)};
      }
    };

    Props<std::tuple<>, std::tuple<>> nullProps() { return {}; };

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_PROPERTIES_H
