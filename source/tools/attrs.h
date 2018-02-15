#ifndef EMP_PLOT_EMP
#define EMP_PLOT_EMP

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "flow.h"
#include "meta/type_traits.h"

namespace emp {
  namespace tools {

    struct value_tag {};

    template <typename V1, typename V2>
    struct ValuesEq
      : std::is_same<typename V1::attr_type, typename V2::attr_type> {};

#define DEFINE_ATTR(NAME, _name)                                               \
  struct NAME {                                                                \
    template <class T>                                                         \
    struct Value : emp::tools::value_tag {                                     \
      static constexpr auto name = #_name;                                     \
      using attr_type = NAME;                                                  \
      using value_type = T;                                                    \
      T _name;                                                                 \
      Value() = delete;                                                        \
      constexpr Value(const T& value) : _name(value) {}                        \
      constexpr Value(T&& value) : _name(std::move(value)) {}                  \
      constexpr Value(const Value& other) : _name(other._name) {}              \
      constexpr Value(Value&& other) : _name(std::move(other._name)) {}        \
      Value& operator=(const T& value) {                                       \
        _name = value;                                                         \
        return *this;                                                          \
      }                                                                        \
      Value& operator=(T&& value) {                                            \
        _name = std::move(value);                                              \
        return *this;                                                          \
      }                                                                        \
      Value& operator=(const Value& other) {                                   \
        if (this != &other) _name = other._name;                               \
        return *this;                                                          \
      }                                                                        \
      Value& operator=(Value&& other) {                                        \
        if (this != &other) _name = std::move(other._name);                    \
        return *this;                                                          \
      }                                                                        \
      constexpr auto get() const { return _name; }                             \
      static constexpr auto get(const Value& value) { return value._name; }    \
      template <class D>                                                       \
      static constexpr auto getOr(const Value& value, D&&) {                   \
        return value._name;                                                    \
      }                                                                        \
      template <class U, class D>                                              \
      static constexpr auto getOr(U&&, D&& value) {                            \
        return std::forward<D>(value);                                         \
      }                                                                        \
      template <class D>                                                       \
      static constexpr auto getOrElse(const Value& value, D&&) {               \
        return value._name;                                                    \
      }                                                                        \
      template <class U, class D>                                              \
      static constexpr auto getOrElse(U&&, D&& value) {                        \
        return std::forward<D>(value)();                                       \
      }                                                                        \
      template <class U>                                                       \
      constexpr auto applyToAttrs(const U& attrs) const {                      \
        return attrs.set(attr_type::value(_name(attrs)));                      \
      }                                                                        \
      template <class Iter>                                                    \
      auto applyToRange(Iter begin, Iter end) const {                          \
        std::vector<decltype(applyToAttrs(*begin))> result;                    \
        for (; begin != end; ++begin) {                                        \
          result.push_back(applyToAttrs(*begin));                              \
        }                                                                      \
        return result;                                                         \
      }                                                                        \
    };                                                                         \
    static constexpr auto name = #_name;                                       \
    template <class T>                                                         \
    static constexpr Value<T>& getValue(Value<T>& value) {                     \
      return value;                                                            \
    }                                                                          \
    template <class T>                                                         \
    static constexpr const Value<T>& getValue(const Value<T>& value) {         \
      return value;                                                            \
    }                                                                          \
    template <class T>                                                         \
    static constexpr auto get(const Value<T>& target) {                        \
      return target._name;                                                     \
    }                                                                          \
                                                                               \
    private:                                                                   \
    template <class T, class... U>                                             \
    static constexpr decltype(auto) callOrGet(                                 \
      const std::true_type& isCallable, T&& target, U&&... args) {             \
      return (std::forward<T>(target)._name)(std::forward<U>(args)...);        \
    }                                                                          \
    template <class T, class... U>                                             \
    static constexpr auto callOrGet(const std::false_type& isCallable,         \
                                    T&& target, U&&... args) {                 \
      return std::forward<T>(target)._name;                                    \
    }                                                                          \
                                                                               \
    public:                                                                    \
    template <class T, class... U>                                             \
    static constexpr decltype(auto) callOrGet(const Value<T>& target,          \
                                              U&&... args) {                   \
      return callOrGet(                                                        \
        emp::is_invocable<T, decltype(std::forward<U>(args))...>{}, target,    \
        std::forward<U>(args)...);                                             \
    }                                                                          \
    template <class T, class... U>                                             \
    static constexpr decltype(auto) callOrGet(Value<T>& target, U&&... args) { \
      return callOrGet(                                                        \
        emp::is_invocable<T, decltype(std::forward<U>(args))...>{}, target,    \
        std::forward<U>(args)...);                                             \
    }                                                                          \
                                                                               \
    private:                                                                   \
    template <class T>                                                         \
    static constexpr std::true_type __in(const Value<T>&) {                    \
      return {};                                                               \
    }                                                                          \
    static constexpr std::false_type __in(...) { return {}; }                  \
                                                                               \
    public:                                                                    \
    template <class T>                                                         \
    struct in : decltype(__in(std::declval<T>())) {};                          \
    template <class T, class D>                                                \
    static constexpr auto getOr(const Value<T>& target, D&&) {                 \
      return get(target);                                                      \
    }                                                                          \
    template <class T, class D>                                                \
    static constexpr auto getOr(                                               \
      T&&, D&& defaultValue,                                                   \
      std::enable_if_t<!in<T>::value, std::nullptr_t> = nullptr) {             \
      return std::forward<D>(defaultValue);                                    \
    }                                                                          \
    template <class T, class D>                                                \
    static constexpr auto getOrElse(const Value<T>& target, D&&) {             \
      return get(target);                                                      \
    }                                                                          \
    template <class T, class D>                                                \
    static constexpr auto getOrElse(                                           \
      T&&, D&& defaultValue,                                                   \
      std::enable_if_t<!in<T>::value, std::nullptr_t> = nullptr) {             \
      return std::forward<D>(defaultValue)();                                  \
    }                                                                          \
    template <class T, class D>                                                \
    static constexpr auto getOrGetIn(const Value<T>& target, D&&) {            \
      return get(target);                                                      \
    }                                                                          \
    template <class T, class D>                                                \
    static constexpr auto getOrGetIn(                                          \
      T&&, D&& defaultValue,                                                   \
      std::enable_if_t<!in<T>::value, std::nullptr_t> = nullptr) {             \
      return get(std::forward<D>(defaultValue));                               \
    }                                                                          \
    template <class T, class V>                                                \
    static constexpr void set(Value<T>& target, V&& value) {                   \
      target._name = std::forward<V>(value);                                   \
    }                                                                          \
    template <class T>                                                         \
    static constexpr Value<std::decay_t<T>> value(T&& value) {                 \
      return {std::forward<T>(value)};                                         \
    }                                                                          \
  };                                                                           \
  template <class T>                                                           \
  constexpr auto _name(T&& value) {                                            \
    return NAME::value(std::forward<T>(value));                                \
  }                                                                            \
  template <class A, class B>                                                  \
  constexpr bool operator==(const NAME::Value<A>& a,                           \
                            const NAME::Value<B>& b) {                         \
    return NAME::get(a) == NAME::get(b);                                       \
  }                                                                            \
  template <class T>                                                           \
  std::ostream& operator<<(std::ostream& out, const NAME::Value<T>& value) {   \
    return out << "\"" #_name "\": " << value._name << std::endl;              \
  }
    template <class...>
    class Attrs;

    template <class>
    struct is_attrs : std::false_type {};

    template <class... U>
    struct is_attrs<Attrs<U...>> : std::true_type {};

    template <class T>
    constexpr auto is_attrs_v{is_attrs<T>::value};

    namespace __attrs_impl {

      template <class... T>
      class AttrsParent : public T... {
        protected:
        struct args_tag {};
        struct copy_tag {};

        template <class U>
        struct __attrs_impl_constructor_detector : args_tag {};

        template <class... U>
        struct __attrs_impl_constructor_detector<Attrs<U...>> : copy_tag {};

        template <class U>
        using constructor_detector =
          __attrs_impl_constructor_detector<std::decay_t<U>>;

        template <class... U>
        constexpr AttrsParent(const args_tag&, U&&... args)
          : T{std::forward<U>(args)}... {}

        template <class... U>
        constexpr AttrsParent(const copy_tag&, const Attrs<U...>& other)
          : T{T::attr_type::get(other)}... {}
      };

    }  // namespace __attrs_impl

    template <class D>
    class ListTransform {
      constexpr D* self() { return static_cast<D*>(this); }
      constexpr const D* self() const { return static_cast<const D*>(this); }
      template <class... U>
      constexpr decltype(auto) callDerived(U&&... args) {
        return (*self())(std::forward<U>(args)...);
      }
      template <class... U>
      constexpr decltype(auto) callDerived(U&&... args) const {
        return (*self())(std::forward<U>(args)...);
      }

      public:
      constexpr ListTransform() = default;
      constexpr ListTransform(const ListTransform&) = default;
      constexpr ListTransform(ListTransform&&) = default;

      template <class InputIter, class OutputIter>
      void apply(InputIter&& begin, InputIter&& end,
                 OutputIter&& target) const {
        std::transform(begin, end, target, [this](auto&& d) {
          return callDerived(std::forward<decltype(d)>(d));
        });
      }

      template <class DataIter, class PrevIter>
      auto apply(DataIter dbegin, DataIter dend, PrevIter pbegin,
                 PrevIter pend) const {
        std::vector<decltype(pbegin->update(callDerived(*dbegin)))> transformed;
        for (; dbegin != dend && pbegin != pend; ++pbegin, ++dbegin) {
          transformed.push_back(pbegin->update(callDerived(*dbegin)));
        }
        return transformed;
      }

      template <class Iter>
      auto apply(Iter begin, Iter end) const {
        std::vector<decltype(callDerived(*begin))> transformed;
        for (; begin != end; ++begin) {
          transformed.push_back(callDerived(*begin));
        }
        return transformed;
      }

      template <class U>
      auto apply(const std::vector<U>& data) const {
        std::vector<decltype(callDerived(data[0]))> transformed;
        transformed.reserve(data.size());

        for (auto& d : data) {
          transformed.push_back(callDerived(d));
        }

        return transformed;
      }
    };

    template <class... T>
    constexpr Attrs<std::decay_t<T>...> attrs(T&&... props);

    template <class... T>
    class Attrs : public Joinable<Attrs<T...>>,
                  public ListTransform<Attrs<T...>>,
                  public __attrs_impl::AttrsParent<T...> {
      public:
      // This is one of the really nasty parts. The problem is that we really
      // want two constructors
      //
      // (1) template<class... U> Attrs(U&& ... attrs),
      // which initializes all the members of the attribute pack and
      //
      // (2) template<class... U> Attrs(const Attrs<U...>& other),
      // which copies the data out of the other attribute pack and into this
      // one.
      //
      // Sadly, C++ can't tell the difference between these two signatures,
      // and will only call the first because of how c++ decides the order to
      // resolve overloads in. In ordered to ensure that the correct
      // constructor is called, we calculate which constructor should be
      // called, and then pass the result into AttrsParent, which has
      // unambiguously overloaded constructors which handle each case
      template <class U0, class... U>
      constexpr Attrs(U0&& arg, U&&... args)
        : __attrs_impl::AttrsParent<T...>(
            // This will resolve to one of several marker structs which will
            // select the correct constructor
            typename __attrs_impl::AttrsParent<
              T...>::template constructor_detector<U0>{},
            // The arguments are then forwarded as normal
            std::forward<U0>(arg), std::forward<U>(args)...) {}

      template <class... U>
      constexpr auto operator()(U... args) {
        return attrs(
          T::attr_type::value(T::attr_type::callOrGet(*this, args...))...);
      }

      template <class... U>
      constexpr auto operator()(U... args) const {
        return attrs(
          T::attr_type::value(T::attr_type::callOrGet(*this, args...))...);
      }

      template <class P>
      constexpr auto set(
        const P& value,  // TODO: this should be a universal reference, but
                         // that creates problems below
        std::enable_if_t<P::attr_type::template in<Attrs>::value,
                         std::nullptr_t> = nullptr) const {
        return Attrs<std::conditional_t<T::attr_type::template in<P>::value,
                                        std::decay_t<P>, T>...>{
          std::decay_t<P>::attr_type::getOr(value, T::get(*this))...};
      }

      template <class P>
      constexpr auto set(
        P&& value,
        std::enable_if_t<!std::decay_t<P>::attr_type::template in<Attrs>::value,
                         std::nullptr_t> = nullptr) const {
        return Attrs<T..., std::decay_t<P>>{T::get(*this)...,
                                            std::forward<P>(value)};
      }

      private:
      // This struct exists so that we can pass around Attrs<U...> without it
      // taking up any memory or having any side effects from construction.
      template <class>
      struct wrapper {};

      template <class P, class... U>
      constexpr auto updateImpl(const P& other,
                                const wrapper<Attrs<U...>>&) const {
        // use the getters to extract all the data
        return attrs(
          U::attr_type::value(U::attr_type::getOrGetIn(other, *this))...);
      }

      public:
      /// Creates a new attribute pack which has all the attributes of this
      /// pack and another pack. Values will be taken from other other pack
      /// preferentially.
      template <class P>
      constexpr decltype(auto) update(const P& other) const {
        return updateImpl(
          other, wrapper<VariadicUnionType<std::decay_t<decltype(*this)>,
                                           std::decay_t<P>>>{});
      }

      private:
      template <class O>
      constexpr bool eq(const O& other, const wrapper<Attrs<>>&) const {
        return true;
      }

      template <class O, class U0, class... U>
      constexpr bool eq(const O& other, const wrapper<Attrs<U0, U...>>&) const {
        return U0::attr_type::get(*this) == U0::attr_type::get(other) &&
               eq(other, wrapper<Attrs<U...>>{});
      }

      public:
      /// Attribute packs support equals and not equals, but are not
      /// orderable, as the members are not considered to be ordered from the
      /// user's perspective.
      template <class... U>
      constexpr bool operator==(const Attrs<U...>& other) const {
        static_assert(sizeof...(T) == sizeof...(U),
                      "Cannot compare attribute packs with different members");
        return eq(other, wrapper<Attrs<U...>>{});
      }
    };
    ///  An alternative syntax for creating attribute packs. Takes any number
    ///  of attributes and returns a pack containing each of those attributes.
    ///  Note that the values will be copied or moved into the attribute pack.
    ///  They will not be referenced.
    template <class... T>
    constexpr Attrs<std::decay_t<T>...> attrs(T&&... props) {
      return {std::forward<T>(props)...};
    };

    template <class T>
    using is_value =
      std::integral_constant<bool, std::is_base_of<value_tag, T>::value &&
                                     !is_attrs_v<T>>;
    template <class T>
    constexpr auto is_value_v = is_value<T>::value;

    template <class From, class To,
              class = std::enable_if_t<is_value_v<From> && is_value_v<To>>>
    constexpr auto operator+(From&& from, To&& to) {
      return attrs(std::forward<From>(from), std::forward<To>(to));
    }

    template <class... From, class To, class = std::enable_if_t<is_value_v<To>>>
    constexpr auto operator+(const Attrs<From...>& from, To&& to) {
      return from.set(to);
    }
    template <class From, class... To,
              class = std::enable_if_t<is_value_v<From>>>
    constexpr auto operator+(From&& from, const Attrs<To...>& to) {
      return to.set(from);
    }

    namespace __attrs_impl {
      template <class...>
      struct print_attrs_tag {};

      template <class... T, class H>
      void printAttrs(std::ostream& out, const Attrs<T...>& attrs,
                      const print_attrs_tag<H>&) {
        out << '"' << H::name << "\": " << H::get(attrs);
      }
      template <class... T, class H0, class H1, class... U>
      void printAttrs(std::ostream& out, const Attrs<T...>& attrs,
                      const print_attrs_tag<H0, H1, U...>&) {
        out << '"' << H0::name << "\": " << H0::get(attrs) << ", ";
        printAttrs(out, attrs, print_attrs_tag<H1, U...>{});
      }
    }  // namespace __attrs_impl

    template <class H, class... T>
    std::ostream& operator<<(std::ostream& out, const Attrs<H, T...>& attrs) {
      out << "{ ";
      __attrs_impl::printAttrs(out, attrs,
                               __attrs_impl::print_attrs_tag<H, T...>{});
      return out << " }";
    }

    std::ostream& operator<<(std::ostream& out, const Attrs<>& attrs) {
      return out << "{ }";
    }
  }  // namespace tools
};  // namespace emp

#endif  // EMP_PLOT_EMP
