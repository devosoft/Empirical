#ifndef PLOT_PROPERTIES_H
#define PLOT_PROPERTIES_H

#include "meta/type_traits.h"

namespace emp {
  namespace plot {

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

    struct properties_tag {};

    template <typename K, typename P>
    class Props;

    template <typename... K, typename... P>
    class Props<std::tuple<K...>, std::tuple<P...>> : public properties_tag {
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

    namespace properties {

      namespace detail {

        template <typename A, typename B>
        class And {
          private:
          A a;
          B b;

          public:
          template <typename A_ = A, typename B_ = B>
          constexpr And(A_&& a, B_&& b)
            : a(std::forward<A_>(a)), b(std::forward<B_>(b)) {}

          template <typename Props>
          constexpr decltype(auto) operator()(Props&& props) const {
            return b(a(std::forward<Props>(props)));
          }

          template <typename Props>
          constexpr decltype(auto) apply(Props&& props) const {
            return (*this)(std::forward<Props>(props));
          }
        };

        template <typename P, typename V>
        class PropertySetter {
          public:
          using property_type = P;
          using value_type = V;
          V value;

          private:
          template <typename Props>
          constexpr decltype(auto) call(Props&& props,
                                        const std::false_type&) const {
            return std::forward<Props>(props).template set<P>(value);
          }

          template <typename Props>
          constexpr decltype(auto) call(Props&& props,
                                        const std::true_type&) const {
            auto v{value(props)};
            return std::forward<Props>(props).template set<P>(v);
          }

          public:
          template <typename Props>
          constexpr decltype(auto) operator()(Props&& props) const {
            static_assert(
              std::is_base_of<properties_tag, std::decay_t<Props>>::value,
              "Attempting to use a property setter on an object which is not a "
              "property bundle!");
            return call(std::forward<Props>(props), is_invocable<V, Props>{});
          }

          template <typename Props>
          constexpr decltype(auto) apply(Props&& props) const {
            return (*this)(std::forward<Props>(props));
          }

          template <typename B>
          constexpr And<PropertySetter, B> andThen(B&& b) const {
            return {*this, std::forward<B>(b)};
          }

          template <typename B>
          constexpr And<PropertySetter, B> operator&&(B&& b) const {
            return {*this, std::forward<B>(b)};
          }
        };

      }  // namespace detail

      template <typename P>
      struct PropertyName {
        private:
        struct Is {
          template <typename V>
          auto operator()(V&& value) const {
            return nullProps().set<P>(std::forward<V>(value));
          };
        };

        struct Get {
          template <typename Props>
          constexpr decltype(auto) operator()(Props&& properties) const {
            static_assert(
              std::is_base_of<properties_tag, std::decay_t<Props>>::value,
              "Expected a property bundle!");
            return std::forward<Props>(properties).template get<P>();
          }
        };

        public:
        template <typename V>
        static constexpr auto to(V&& map) {
          return detail::PropertySetter<P, std::decay_t<V>>{
            std::forward<V>(map)};
        }

        // You may be wondering why these would be defined as callable structs,
        // rather than static methods. This is because static methods which take
        // template arguments cannot be used as like function pointers, unless
        // they are fully specialized. This makes sense because -- until they
        // are fully specialized -- they don't have any address to point to,
        // since they are just abstractions that aren't attached to actual code
        // otherwise. In any case, doing this lets them be used *almost* like
        // they are regular, non-template functions. There is one slight
        // difference, however. Suppose you have some function which takes a
        // callback: foo([](int){ return 0; }).
        // If you have another function
        // int HelloWorld(int) { return 0; }
        // Then you would call foo like this: foo(&HelloWorld). However, if you
        // want to pass PropertyName::get, you would call it like this:
        // foo(PropertyName::get). Notice the lack of '&'. This is missing in
        // the second case because you do not want to take the address of
        // PropertyName::get, you want to pass it by reference.
        static constexpr Is is{};
        static constexpr Get get{};
      };

      struct Fill : PropertyName<Fill> {};
      struct Stroke : PropertyName<Stroke> {};
      struct FillShader : PropertyName<FillShader> {};
      struct StrokeShader : PropertyName<StrokeShader> {};
      struct StrokeWeight : PropertyName<StrokeWeight> {};

      struct CartesianData : PropertyName<CartesianData> {};
      struct CartesianScaled : PropertyName<CartesianScaled> {};

      struct Value : PropertyName<Value> {};
    };  // namespace properties

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_PROPERTIES_H
