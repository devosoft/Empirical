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

    template <typename T>
    struct IsAttributeValue : std::is_base_of<value_tag, T> {};

    namespace __impl_has_attr {
      template <typename Pack, typename Attr>
      struct HasAttr {
        template <typename T>
        static constexpr std::true_type hasAttr(
          const typename Attr::template value_type<T>&) {
          return {};
        }
        static constexpr std::false_type hasAttr(...) { return {}; }

        using type = decltype(hasAttr(std::declval<Pack>()));
      };
    }  // namespace __impl_has_attr

    template <typename Pack, typename Attr>
    struct HasAttr : __impl_has_attr::HasAttr<Pack, Attr>::type {};

    namespace __impl_attr_base {
      template <typename Attr, template <typename> class Value>
      struct AttrBase {
        using attr_type = Attr;
        template <typename T>
        using value_type = Value<T>;

        /// Given an attribute pack, getValue will extract just this attribute
        template <class T>
        static constexpr decltype(auto) getValue(value_type<T>& value) {
          return value;
        }

        template <class T>
        static constexpr decltype(auto) getValue(const value_type<T>& value) {
          return value;
        }

        template <class T>
        static constexpr decltype(auto) getValue(value_type<T>&& value) {
          return value;
        }

        /// Given an attribute pack, get(pack) will extract the value of this
        /// attribute in that pack
        template <class T>
        static constexpr decltype(auto) get(const value_type<T>& target) {
          return target.get();
        }

        template <class T>
        static constexpr decltype(auto) get(value_type<T>& target) {
          return target.get();
        }

        template <class T>
        static constexpr decltype(auto) get(value_type<T>&& target) {
          return target.get();
        }

        // -- callOrGet --
        private:
        // Handle the case when get(target) is callable
        template <class Pack, class... U>
        static constexpr decltype(auto) __impl_callOrGet(
          const std::true_type& isCallable, Pack&& pack, U&&... args) {
          return get(std::forward<Pack>(pack))(std::forward<U>(args)...);
        }

        // Handle the case when get(target) is not callable
        template <class T, class... U>
        static constexpr decltype(auto) __impl_callOrGet(
          const std::false_type& isCallable, T&& target, U&&... args) {
          return get(std::forward<T>(target));
        }

        public:
        /// Given an attribute pack, callOrGet(pack, args...) will attempt to
        /// return get(pack)(args...). If this fails to compile, because the
        /// value of this attribute is not callable in the given pack, then just
        /// the value of this attribute will be returned, get(pack).
        template <class V, class... U>
        static constexpr decltype(auto) callOrGet(V&& target, U&&... args) {
          using ValueOfTargetType = decltype(get(std::forward<V>(target)));

          return __impl_callOrGet(
            // Check if the target attribute value is invocable
            emp::is_invocable<ValueOfTargetType,
                              decltype(std::forward<U>(args))...>{},
            std::forward<V>(target), std::forward<U>(args)...);
        }
        // -- getOrElse --
        private:
        template <class Pack, class F, class... U>
        static constexpr decltype(auto) __impl_getOrElse(const std::true_type&,
                                                         Pack&& pack, F&&,
                                                         U&&...) {
          return get(std::forward<Pack>(pack));
        }

        template <class Pack, class F, class... U>
        static constexpr decltype(auto) __impl_getOrElse(const std::false_type&,
                                                         Pack&&,
                                                         F&& defaultFunction,
                                                         U&&... args) {
          return std::forward<F>(defaultFunction)(std::forward<U>(args)...);
        }

        public:
        template <class Pack, class F, class... U>
        static constexpr decltype(auto) getOrElse(Pack&& pack,
                                                  F&& defaultFunction,
                                                  U&&... args) {
          return __impl_getOrElse(
            HasAttr<std::decay_t<Pack>, attr_type>{}, std::forward<Pack>(pack),
            std::forward<F>(defaultFunction), std::forward<U>(args)...);
        }

        private:
        // Utility class used by getOr which simply remembers and returns a
        // value. @todo: when we switch to c++17, we can replace this with a
        // constexpr closure
        template <class D>
        struct Default {
          D value;
          constexpr decltype(auto) operator()() const {
            return std::forward<D>(value);
          }
        };

        public:
        template <class Pack, class D>
        static constexpr decltype(auto) getOr(Pack&& pack, D&& defaultValue) {
          return getOrElse(std::forward<Pack>(pack),
                           Default<D&&>{std::forward<D>(defaultValue)});
        }

        private:
        // Utility class used by getOrGetIn which simply remembers a attribute
        // pack.
        template <typename...>
        struct FallbackHandler;

        template <typename Fallback>
        struct FallbackHandler<Fallback> {
          Fallback fallback;
          template <typename F>
          constexpr FallbackHandler(F&& fallback)
            : fallback(std::forward<F>(fallback)) {}

          constexpr decltype(auto) operator()() const {
            return get(std::forward<Fallback>(fallback));
          }
        };

        template <typename Fallback0, typename Fallback1, typename... Fallbacks>
        struct FallbackHandler<Fallback0, Fallback1, Fallbacks...> {
          Fallback0 fallback;
          FallbackHandler<Fallback1, Fallbacks...> fallbacks;
          template <typename F0, typename F1, typename... F>
          constexpr FallbackHandler(F0&& fallback0, F1&& fallback1,
                                    F&&... fallbacks)
            : fallback(std::forward<F0>(fallback0)),
              fallbacks(std::forward<F1>(fallback1),
                        std::forward<F>(fallbacks)...) {}

          constexpr decltype(auto) operator()() const {
            return getOrElse(fallback, fallbacks);
          }
        };

        public:
        template <class... Fallbacks>
        static constexpr decltype(auto) getOrGetIn(Fallbacks&&... fallbacks) {
          return FallbackHandler<Fallbacks&&...>{
            std::forward<Fallbacks>(fallbacks)...}();
        }

        template <class T, class V>
        static constexpr void set(value_type<T>& target, V&& value) {
          get(target) = std::forward<V>(value);
        }
        template <class T>
        static constexpr value_type<T> value(T&& value) {
          return {std::forward<T>(value)};
        }
      };
    };  // namespace __impl_attr_base

#define DEFINE_ATTR(NAME, _name)                                             \
  struct NAME;                                                               \
  template <class T>                                                         \
  struct NAME##Value : emp::tools::value_tag {                               \
    static constexpr auto name = #_name;                                     \
    using attr_type = NAME;                                                  \
    using value_type = T;                                                    \
    T _name;                                                                 \
    NAME##Value() = delete;                                                  \
    constexpr NAME##Value(const T& value) : _name(value) {}                  \
    constexpr NAME##Value(T&& value) : _name(std::move(value)) {}            \
    constexpr NAME##Value(const NAME##Value& other) : _name(other._name) {}  \
    constexpr NAME##Value(NAME##Value&& other)                               \
      : _name(std::move(other._name)) {}                                     \
    constexpr NAME##Value& operator=(const T& value) {                       \
      _name = value;                                                         \
      return *this;                                                          \
    }                                                                        \
    constexpr NAME##Value& operator=(T&& value) {                            \
      _name = std::move(value);                                              \
      return *this;                                                          \
    }                                                                        \
    constexpr NAME##Value& operator=(const NAME##Value& other) {             \
      if (this != &other) _name = other._name;                               \
      return *this;                                                          \
    }                                                                        \
    constexpr NAME##Value& operator=(NAME##Value&& other) {                  \
      if (this != &other) _name = std::move(other._name);                    \
      return *this;                                                          \
    }                                                                        \
    constexpr T& get() & { return _name; }                                   \
    constexpr T&& get() && { return std::move(_name); }                      \
    constexpr const T& get() const & { return _name; }                       \
    constexpr const T& get() const && { return std::move(_name); }           \
  };                                                                         \
  struct NAME : emp::tools::__impl_attr_base::AttrBase<NAME, NAME##Value> {  \
    static constexpr auto name = #_name;                                     \
  };                                                                         \
  template <class T>                                                         \
  constexpr auto _name(T&& value) {                                          \
    return NAME::value(std::forward<T>(value));                              \
  }                                                                          \
  template <class A, class B>                                                \
  constexpr bool operator==(const NAME##Value<A>& a,                         \
                            const NAME##Value<B>& b) {                       \
    return NAME::get(a) == NAME::get(b);                                     \
  }                                                                          \
  template <class T>                                                         \
  std::ostream& operator<<(std::ostream& out, const NAME##Value<T>& value) { \
    return out << "\"" #_name "\": " << value._name << std::endl;            \
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

      private:
      template <bool Last, typename A>
      struct GetAttr;
      template <typename A>
      struct GetAttr<false, A> {
        template <typename S, typename... U>
        static constexpr decltype(auto) get(S&& self, U&&... args) {
          using attr_type = typename A::attr_type;
          return attr_type::value(attr_type::callOrGet(self, args...));
        }
      };

      template <typename A>
      struct GetAttr<true, A> {
        template <typename S, typename... U>
        static constexpr decltype(auto) get(S&& self, U&&... args) {
          using attr_type = typename A::attr_type;
          return attr_type::value(attr_type::callOrGet(
            std::forward<S>(self), std::forward<U>(args)...));
        }
      };

      template <typename S, typename... U>
      static constexpr decltype(auto) call(S&& self, U&&... args) {
        constexpr size_t last = sizeof...(T) - 1;
        return attrs(
          GetAttr<VariadicIndexOfValue<T, Attrs> == last,
                  T>::template get<S, U...>(std::forward<S>(self),
                                            std::forward<U>(args)...)...);
      }

      public:
      template <class... U>
      constexpr auto operator()(U&&... args) & {
        return call(*this, std::forward<U>(args)...);
      }

      template <class... U>
      constexpr auto operator()(U&&... args) && {
        return call(std::move(*this), std::forward<U>(args)...);
      }

      template <class... U>
      constexpr auto operator()(U&&... args) const & {
        return call(*this, std::forward<U>(args)...);
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
        std::enable_if_t<
          !HasAttr<Attrs, typename std::decay_t<P>::attr_type>::value,
          std::nullptr_t> = nullptr) const {
        return Attrs<T..., std::decay_t<P>>{T::attr_type::get(*this)...,
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
