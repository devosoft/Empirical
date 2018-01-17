#ifndef EMP_PLOT_EMP
#define EMP_PLOT_EMP

#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

#include "meta/type_traits.h"

namespace emp {
  namespace plot {

#define DEFINE_PROPERTY(NAME, _name)                                         \
  struct NAME {                                                              \
    template <class T>                                                       \
    struct Value {                                                           \
      static constexpr auto name = #_name;                                   \
      using property_type = NAME;                                            \
      using value_type = T;                                                  \
      T _name;                                                               \
      Value() = delete;                                                      \
      Value(const T& value) : _name(value) {}                                \
      Value(T&& value) : _name(std::move(value)) {}                          \
      Value(const Value& other) : _name(other._name) {}                      \
      Value(Value&& other) : _name(std::move(other._name)) {}                \
      Value& operator=(const T& value) {                                     \
        _name = value;                                                       \
        return *this;                                                        \
      }                                                                      \
      Value& operator=(T&& value) {                                          \
        _name = std::move(value);                                            \
        return *this;                                                        \
      }                                                                      \
      Value& operator=(const Value& other) {                                 \
        if (this != &other) _name = other._name;                             \
        return *this;                                                        \
      }                                                                      \
      Value& operator=(Value&& other) {                                      \
        if (this != &other) _name = std::move(other._name);                  \
        return *this;                                                        \
      }                                                                      \
      constexpr auto get() const { return _name; }                           \
      static constexpr auto get(const Value& value) { return value._name; }  \
      template <class D>                                                     \
      static constexpr auto getOr(const Value& value, D&&) {                 \
        return value._name;                                                  \
      }                                                                      \
      template <class U, class D>                                            \
      static constexpr auto getOr(U&&, D&& value) {                          \
        return std::forward<D>(value);                                       \
      }                                                                      \
      template <class D>                                                     \
      static constexpr auto getOrElse(const Value& value, D&&) {             \
        return value._name;                                                  \
      }                                                                      \
      template <class U, class D>                                            \
      static constexpr auto getOrElse(U&&, D&& value) {                      \
        return std::forward<D>(value)();                                     \
      }                                                                      \
    };                                                                       \
    static constexpr auto name = #_name;                                     \
    template <class T>                                                       \
    static constexpr Value<T>& getValue(Value<T>& value) {                   \
      return value;                                                          \
    }                                                                        \
    template <class T>                                                       \
    static constexpr const Value<T>& getValue(const Value<T>& value) {       \
      return value;                                                          \
    }                                                                        \
    template <class T>                                                       \
    static constexpr auto get(const Value<T>& target) {                      \
      return target._name;                                                   \
    }                                                                        \
                                                                             \
    private:                                                                 \
    template <class T>                                                       \
    static constexpr std::true_type __in(const Value<T>&) {                  \
      return {};                                                             \
    }                                                                        \
    static constexpr std::false_type __in(...) { return {}; }                \
                                                                             \
    public:                                                                  \
    template <class T>                                                       \
    struct in : decltype(__in(std::declval<T>())) {};                        \
    template <class T, class D>                                              \
    static constexpr auto getOr(const Value<T>& target, D&&) {               \
      return get(target);                                                    \
    }                                                                        \
    template <class T, class D>                                              \
    static constexpr auto getOr(                                             \
      T&&, D&& defaultValue,                                                 \
      std::enable_if_t<!in<T>::value, std::nullptr_t> = nullptr) {           \
      return std::forward<D>(defaultValue);                                  \
    }                                                                        \
    template <class T, class D>                                              \
    static constexpr auto getOrElse(const Value<T>& target, D&&) {           \
      return get(target);                                                    \
    }                                                                        \
    template <class T, class D>                                              \
    static constexpr auto getOrElse(                                         \
      T&&, D&& defaultValue,                                                 \
      std::enable_if_t<!in<T>::value, std::nullptr_t> = nullptr) {           \
      return std::forward<D>(defaultValue)();                                \
    }                                                                        \
    template <class T, class V>                                              \
    static constexpr void set(Value<T>& target, V&& value) {                 \
      target._name = std::forward<V>(value);                                 \
    }                                                                        \
    template <class T>                                                       \
    static constexpr Value<std::decay_t<T>> value(T&& value) {               \
      return {std::forward<T>(value)};                                       \
    }                                                                        \
  };                                                                         \
  template <class T>                                                         \
  auto _name(T&& value) {                                                    \
    return NAME::value(std::forward<T>(value));                              \
  }                                                                          \
  template <class T>                                                         \
  std::ostream& operator<<(std::ostream& out, const NAME::Value<T>& value) { \
    return out << "{ " << value._name << " }" << std::endl;                  \
  }
    template <class...>
    class Properties;

    template <class>
    struct is_properties : std::false_type {};

    template <class... U>
    struct is_properties<Properties<U...>> : std::true_type {};

    template <class T>
    constexpr auto is_properties_v{is_properties<T>::value};

    namespace __impl {

      template <class T, class... A>
      struct vardic_pack_contains : std::false_type {};
      template <class T, class... A>
      struct vardic_pack_contains<T, T, A...> : std::true_type {};
      template <class T, class A0, class... A>
      struct vardic_pack_contains<T, A0, A...> : vardic_pack_contains<T, A...> {
      };

      template <class...>
      struct no_repeats_in_vardic_pack : std::true_type {};
      template <class T0, class... T>
      struct no_repeats_in_vardic_pack<T0, T...>
        : std::conditional_t<vardic_pack_contains<T0, T...>::value,
                             no_repeats_in_vardic_pack<T...>, std::false_type> {
      };

      template <class... T>
      class PropertiesParent : public T... {
        protected:
        struct args_tag {};
        struct copy_tag {};

        template <class U>
        struct constructor_detector : args_tag {};

        template <class... U>
        struct constructor_detector<Properties<U...>> : copy_tag {};

        template <class... U>
        constexpr PropertiesParent(const args_tag&, U&&... args)
          : T{std::forward<U>(args)}... {}

        template <class... U>
        constexpr PropertiesParent(const copy_tag&,
                                   const Properties<U...>& other)
          : T{T::property_type::get(other)}... {}
      };

    }  // namespace __impl

    template <class D>
    class ListTransform {
      public:
      template <class InputIter, class OutputIter>
      void transform(InputIter&& begin, InputIter&& end,
                     OutputIter&& target) const {
        std::transform(begin, end, target, static_cast<const D&>(*this));
      }

      template <class Iter>
      auto transform(Iter&& begin, Iter&& end) const {
        std::vector<decltype((*this)(*begin))> transformed;
        std::transform(std::forward<Iter>(begin), std::forward<Iter>(end),
                       std::back_inserter(transformed),
                       static_cast<const D&>(*this));
        return transformed;
      }

      template <class U>
      auto transform(const std::vector<U>& data) const {
        std::vector<decltype((*this)(data[0]))> transformed;
        transformed.reserve(data.size());

        for (auto& d : data) {
          transformed.push_back(static_cast<const D&>(*this)(d));
        }

        return transformed;
      }
    };

    template <class... T>
    Properties<std::decay_t<T>...> properties(T&&... props);

    template <class... T>
    class Properties : public ListTransform<Properties<T...>>,
                       public __impl::PropertiesParent<T...> {
      public:
      template <class U0, class... U>
      constexpr Properties(U0&& arg, U&&... args)
        : __impl::PropertiesParent<T...>(
            typename __impl::PropertiesParent<
              T...>::template constructor_detector<std::decay_t<U0>>{},
            std::forward<U0>(arg), std::forward<U>(args)...) {}

      template <class... U>
      constexpr auto operator()(U... args) {
        return properties(T::property_type::value(T::get(*this)(args...))...);
      }

      template <class... U>
      constexpr auto operator()(U... args) const {
        return properties(T::property_type::value(T::get(*this)(args...))...);
      }

      template <class P>
      auto set(
        const P& value,  // TODO: this should be a universal reference, but that
                         // creates problems below
        std::enable_if_t<P::property_type::template in<Properties>::value,
                         std::nullptr_t> = nullptr) const {
        return Properties<
          std::conditional_t<T::property_type::template in<P>::value, P, T>...>{
          P::property_type::getOr(value, T::get(*this))...};
      }
      template <class P>
      auto set(
        P&& value,
        std::enable_if_t<!P::property_type::template in<Properties>::value,
                         std::nullptr_t> = nullptr) const {
        return Properties<T..., std::decay_t<P>>{T::get(*this)...,
                                                 std::forward<P>(value)};
      }
    };
    template <class... T>
    Properties<std::decay_t<T>...> properties(T&&... props) {
      return {std::forward<T>(props)...};
    };

    namespace __impl {
      template <class...>
      struct print_properties_tag {};

      template <class... T, class H>
      void printProperties(std::ostream& out,
                           const Properties<T...>& properties,
                           const print_properties_tag<H>&) {
        out << H::name << ": " << H::get(properties);
      }
      template <class... T, class H0, class H1, class... U>
      void printProperties(std::ostream& out,
                           const Properties<T...>& properties,
                           const print_properties_tag<H0, H1, U...>&) {
        out << H0::name << ": " << H0::get(properties) << ", ";
        printProperties(out, properties, print_properties_tag<H1, U...>{});
      }
    }  // namespace __impl

    template <class H, class... T>
    std::ostream& operator<<(std::ostream& out,
                             const Properties<H, T...>& properties) {
      out << "{ ";
      __impl::printProperties(out, properties,
                              __impl::print_properties_tag<H, T...>{});
      return out << " }";
    }

    std::ostream& operator<<(std::ostream& out,
                             const Properties<>& properties) {
      return out << "{ }";
    }

    DEFINE_PROPERTY(XY, xy);
    DEFINE_PROPERTY(XYScaled, xyScaled);

    template <class I, class M>
    class ScaleData {
      public:
      using input_type = I;
      using map_type = M;
      using output_type = decltype(std::declval<M>()(std::declval<I>()));

      private:
      map_type map;
      std::vector<output_type> data;

      public:
      template <class Iter>
      ScaleData& push(Iter begin, Iter end) {
        map.transform(begin, end, std::back_inserter(data));
        return *this;
      }

      ScaleData& clear() {
        data.clear();
        return *this;
      }
    };

  }  // namespace plot
};  // namespace emp

#endif  // EMP_PLOT_EMP
