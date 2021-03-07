#ifndef EMP_PLOT_EMP
#define EMP_PLOT_EMP

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "../meta/TypePack.hpp"
#include "../meta/type_traits.hpp"

namespace emp {
  namespace tools {

    struct value_tag {};

    template <typename V1, typename V2>
    struct is_same_attribute
      : std::is_same<typename V1::attribute_t, typename V2::attribute_t> {};

    template <typename T>
    struct is_attribute_value : std::is_base_of<value_tag, T> {};
    template <typename T>
    constexpr bool is_attribute_value_v = is_attribute_value<T>::value;

    template <typename...>
    class Attrs;

    template <typename T>
    struct is_attributes_pack : std::false_type {};
    template <typename... U>
    struct is_attributes_pack<Attrs<U...>> : std::true_type {};

    template <typename T>
    constexpr bool is_attributes_pack_v = is_attributes_pack<T>::value;

    namespace __impl_has_attr {
      template <typename Pack, typename Attr>
      struct has_attribute {
        template <typename T>
        static constexpr std::true_type HasAttr(
          const typename Attr::template value_t<T>&) {
          return {};
        }
        static constexpr std::false_type HasAttr(...) { return {}; }

        using type = decltype(HasAttr(std::declval<Pack>()));
      };
    }  // namespace __impl_has_attr

    template <typename Pack, typename Attr>
    struct has_attribute : __impl_has_attr::has_attribute<Pack, Attr>::type {};
    template <typename Pack, typename Attr>
    constexpr bool has_attribute_v = has_attribute<Pack, Attr>::value;

    namespace __impl_attr_base {
      template <typename Attr, template <typename> class Value>
      struct AttrBase {
        using attribute_t = Attr;
        template <typename T>
        using value_t = Value<T>;

        static constexpr struct get_attribute_t {
          /// Given an attribute pack, GetAttribute will extract just this
          /// attribute
          template <class T>
          constexpr decltype(auto) operator()(value_t<T>& value) const {
            return value;
          }

          template <class T>
          constexpr decltype(auto) operator()(const value_t<T>& value) const {
            return value;
          }

          template <class T>
          constexpr decltype(auto) operator()(value_t<T>&& value) const {
            return std::move(value);
          }

          template <class T>
          constexpr decltype(auto) operator()(const value_t<T>&& value) const {
            return std::move(value);
          }
        } GetAttribute{};

        /// Given an attribute pack, Get(pack) will extract the value of this
        /// attribute in that pack. It lives in this strange container struct
        /// because that allows it to be passed into mapping function functions,
        /// so you can do something like:
        /// <code>
        /// std::vector<SomeAttributePack> packs;
        /// std::transform(packs.begin(), packs.end(), MyAttr::Get);
        /// </code>
        static constexpr struct get_t {
          template <class T>
          constexpr const T& operator()(const value_t<T>& target) const {
            return *target;
          }

          template <class T>
          constexpr T& operator()(value_t<T>& target) const {
            return *target;
          }

          template <class T>
          constexpr T&& operator()(value_t<T>&& target) const {
            return *std::move(target);
          }

          template <class T>
          constexpr const T&& operator()(const value_t<T>&& target) const {
            return *std::move(target);
          }
        } Get{};

        // -- CallOrGet --
        private:
        // Handle the case when Get(target) is callable
        template <class T, class... U>
        static constexpr auto __impl_CallOrGetAttribute(
          const std::true_type& isCallable, T&& target, U&&... args) {
          return Make(Get(std::forward<T>(target))(std::forward<U>(args)...));
        }

        // Handle the case when Get(target) is not callable
        template <class T, class... U>
        static constexpr auto __impl_CallOrGetAttribute(
          const std::false_type& isCallable, T&& target, U&&... args) {
          return GetAttribute(std::forward<T>(target));
        }

        public:
        /// Given an attribute pack, CallOrGetAttribute(pack, args...) will
        /// attempt to return Get(pack)(args...). If this fails to compile,
        /// because the value of this attribute is not callable in the given
        /// pack, then just this attribute will be returned, Get(pack).
        static constexpr struct call_or_get_attribute_t {
          template <class V, class... U>
          constexpr decltype(auto) operator()(V&& target, U&&... args) const {
            using ValueOfTargetType = decltype(Get(std::forward<V>(target)));

            return __impl_CallOrGetAttribute(
              // Check if the target attribute value is invocable
              emp::is_invocable<ValueOfTargetType,
                                decltype(std::forward<U>(args))...>{},
              std::forward<V>(target), std::forward<U>(args)...);
          }
        } CallOrGetAttribute{};

        // -- GetOrElse --
        private:
        template <typename Pack, typename M, typename D, typename... U>
        static constexpr decltype(auto) __impl_MapOrElse(const std::true_type&,
                                                         Pack&& pack, M&& map,
                                                         D&&, U&&...) {
          return std::forward<M>(map)(GetAttribute(std::forward<Pack>(pack)));
        }

        template <typename Pack, typename M, typename D, typename... U>
        static constexpr decltype(auto) __impl_MapOrElse(const std::false_type&,
                                                         Pack&&, M&&,
                                                         D&& defaultFunction,
                                                         U&&... args) {
          return std::forward<D>(defaultFunction)(std::forward<U>(args)...);
        }

        public:
        static constexpr struct map_or_else_t {
          template <typename Pack, typename M, typename D, typename... U>
          constexpr decltype(auto) operator()(Pack&& pack, M&& map,
                                              D&& defaultFunction,
                                              U&&... args) const {
            return __impl_MapOrElse(
              has_attribute<std::decay_t<Pack>, attribute_t>{},
              std::forward<Pack>(pack), std::forward<M>(map),
              std::forward<D>(defaultFunction), std::forward<U>(args)...);
          }
        } MapOrElse{};

        static constexpr struct get_attribute_or_else_t {
          template <typename Pack, typename D, typename... U>
          constexpr decltype(auto) operator()(Pack&& pack, D&& defaultFunction,
                                              U&&... args) const {
            return MapOrElse(std::forward<Pack>(pack), GetAttribute,
                             std::forward<D>(defaultFunction),
                             std::forward<U>(args)...);
          }
        } GetAttributeOrElse{};

        static constexpr struct get_or_else_t {
          template <class Pack, class F, class... U>
          constexpr decltype(auto) operator()(Pack&& pack, F&& defaultFunction,
                                              U&&... args) const {
            return MapOrElse(std::forward<Pack>(pack), Get,
                             std::forward<F>(defaultFunction),
                             std::forward<U>(args)...);
          }
        } GetOrElse{};

        private:
        // Utility class used by GetOr which simply remembers and returns a
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
        static constexpr struct get_or_t {
          template <class Pack, class D>
          constexpr decltype(auto) operator()(Pack&& pack,
                                              D&& defaultValue) const {
            return GetOrElse(std::forward<Pack>(pack),
                             Default<D&&>{std::forward<D>(defaultValue)});
          }
        } GetOr{};

        private:
        // Utility class used by GetOrGetIn which simply remembers a attribute
        // pack.
        template <typename...>
        struct FallbackHandler;

        template <typename Fallback>
        struct FallbackHandler<Fallback> {
          Fallback fallback;
          constexpr FallbackHandler(Fallback fallback)
            : fallback(std::forward<Fallback>(fallback)) {}

          constexpr decltype(auto) operator()() const {
            return GetAttribute(std::forward<Fallback>(fallback));
          }
        };

        template <typename Fallback0, typename Fallback1, typename... Fallbacks>
        struct FallbackHandler<Fallback0, Fallback1, Fallbacks...> {
          Fallback0 fallback;
          FallbackHandler<Fallback1, Fallbacks...> fallbacks;
          constexpr FallbackHandler(Fallback0&& fallback0,
                                    Fallback1&& fallback1,
                                    Fallbacks&&... fallbacks)
            : fallback(std::forward<Fallback0>(fallback0)),
              fallbacks(std::forward<Fallback1>(fallback1),
                        std::forward<Fallbacks>(fallbacks)...) {}

          constexpr decltype(auto) operator()() && {
            return GetAttributeOrElse(std::forward<Fallback0>(fallback),
                                      std::move(fallbacks));
          }
        };

        public:
        static constexpr struct get_attribute_or_get_attribute_in_t {
          template <class... Fallbacks>
          constexpr decltype(auto) operator()(Fallbacks&&... fallbacks) const {
            return FallbackHandler<Fallbacks&&...>{
              std::forward<Fallbacks>(fallbacks)...}();
          }
        } GetAttributeOrGetAttributeIn{};

        static constexpr struct get_or_get_in_t {
          template <class... Fallbacks>
          constexpr decltype(auto) operator()(Fallbacks&&... fallbacks) const {
            return Get(FallbackHandler<Fallbacks&&...>{
              std::forward<Fallbacks>(fallbacks)...}());
          }
        } GetOrGetIn{};
        static constexpr struct set_t {
          template <class T, class V>
          constexpr void operator()(value_t<T>& target, V&& value) const {
            Get(target) = std::forward<V>(value);
          }
        } Set{};

        static constexpr struct make_t {
          template <class T>
          constexpr value_t<std::decay_t<T>> operator()(T&& value) const {
            return {std::forward<T>(value)};
          }
        } Make{};
      };

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::get_attribute_t
        AttrBase<Attr, Value>::GetAttribute;

      template <typename Attr, template <typename> class Value>
      constexpr
        typename AttrBase<Attr, Value>::get_t AttrBase<Attr, Value>::Get;

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::call_or_get_attribute_t
        AttrBase<Attr, Value>::CallOrGetAttribute;

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::map_or_else_t
        AttrBase<Attr, Value>::MapOrElse;

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::get_attribute_or_else_t
        AttrBase<Attr, Value>::GetAttributeOrElse;

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::get_or_else_t
        AttrBase<Attr, Value>::GetOrElse;

      template <typename Attr, template <typename> class Value>
      constexpr
        typename AttrBase<Attr, Value>::get_or_t AttrBase<Attr, Value>::GetOr;

      template <typename Attr, template <typename> class Value>
      constexpr
        typename AttrBase<Attr, Value>::get_attribute_or_get_attribute_in_t
          AttrBase<Attr, Value>::GetAttributeOrGetAttributeIn;

      template <typename Attr, template <typename> class Value>
      constexpr typename AttrBase<Attr, Value>::get_or_get_in_t
        AttrBase<Attr, Value>::GetOrGetIn;

      template <typename Attr, template <typename> class Value>
      constexpr
        typename AttrBase<Attr, Value>::set_t AttrBase<Attr, Value>::Set;

      template <typename Attr, template <typename> class Value>
      constexpr
        typename AttrBase<Attr, Value>::make_t AttrBase<Attr, Value>::Make;

    }  // namespace __impl_attr_base

#define DEFINE_ATTR(NAME)                                                      \
  template <class T>                                                           \
  struct NAME##Value;                                                          \
  constexpr struct NAME                                                        \
    : emp::tools::__impl_attr_base::AttrBase<NAME, NAME##Value> {              \
    static constexpr auto name = #NAME;                                        \
    template <typename T>                                                      \
    constexpr value_t<std::decay_t<T>> operator()(T&& value) const {           \
      return {std::forward<T>(value)};                                         \
    }                                                                          \
  } NAME;                                                                      \
  constexpr const char* NAME::name;                                            \
  template <class T>                                                           \
  struct NAME##Value : emp::tools::value_tag, NAME {                           \
    private:                                                                   \
    T value;                                                                   \
                                                                               \
    public:                                                                    \
    static constexpr auto name = #NAME;                                        \
    using attribute_t = struct NAME;                                           \
    using attributes_t = emp::tools::Attrs<NAME##Value<T>>;                    \
    using value_t = T;                                                         \
    NAME##Value() = delete;                                                    \
    constexpr NAME##Value(const T& value) : value(value) {}                    \
    constexpr NAME##Value(T&& value) : value(std::move(value)) {}              \
    template <typename U = T>                                                  \
    constexpr NAME##Value(const NAME##Value<U>& other) : value(*other) {}      \
    template <typename U = T>                                                  \
    constexpr NAME##Value(NAME##Value<U>&& other)                              \
      : value(*std::move(other)) {}                                            \
    constexpr NAME##Value& operator=(const T& value) {                         \
      /* Don't check for self assignment, and assume that value will handle it \
       * correcly */                                                           \
      this->value = value;                                                     \
      return *this;                                                            \
    }                                                                          \
    constexpr NAME##Value& operator=(T&& value) {                              \
      /* Assume value handles self assigment correctly */                      \
      this->value = std::move(value);                                          \
      return *this;                                                            \
    }                                                                          \
    template <typename U = T>                                                  \
    constexpr NAME##Value& operator=(const NAME##Value<U>& other) {            \
      /* Assume value handles self assigment correctly */                      \
      value = *other;                                                          \
      return *this;                                                            \
    }                                                                          \
    template <typename U = T>                                                  \
    constexpr NAME##Value& operator=(NAME##Value<U>&& other) {                 \
      /* Assume value handles self assigment correctly */                      \
      value = *std::move(other);                                               \
      return *this;                                                            \
    }                                                                          \
    constexpr const T& Get##NAME() const & { return value; }                   \
    constexpr const T&& Get##NAME() const && { return std::move(value); }      \
    constexpr const T& Get() const & { return value; }                         \
    constexpr const T&& Get() const && { return std::move(value); }            \
    template <typename U = T>                                                  \
    constexpr const NAME##Value& Set##NAME(U&& value) {                        \
      this->value = std::forward<U>(value);                                    \
      return *this;                                                            \
    }                                                                          \
    template <typename U = T>                                                  \
    constexpr const NAME##Value& Set(U&& value) {                              \
      return Set##NAME(std::forward<U>(value));                                \
    }                                                                          \
    template <typename U = T>                                                  \
    constexpr const NAME##Value& Set(const NAME##Value<U>& value) {            \
      return Set##NAME(*value);                                                \
    }                                                                          \
    template <typename U = T>                                                  \
    constexpr const NAME##Value& Set(NAME##Value<U>&& value) {                 \
      return Set##NAME(*std::move(value));                                     \
    }                                                                          \
    constexpr T& operator*() & { return value; }                               \
    constexpr T&& operator*() && { return std::move(value); }                  \
    constexpr const T& operator*() const & { return value; }                   \
    constexpr const T&& operator*() const && { return std::move(value); }      \
    template <typename M>                                                      \
    constexpr auto Map(M&& map) & {                                            \
      return attribute_t::Make(std::forward<M>(value));                        \
    }                                                                          \
    template <typename M>                                                      \
    constexpr auto Map(M&& map) const & {                                      \
      return attribute_t::Make(std::forward<M>(value));                        \
    }                                                                          \
    template <typename M>                                                      \
    constexpr auto Map(M&& map) && {                                           \
      return attribute_t::Make(std::forward<M>(std::move(value)));             \
    }                                                                          \
    template <typename M>                                                      \
    constexpr auto Map(M&& map) const && {                                     \
      return attribute_t::Make(std::forward<M>(std::move(value)));             \
    }                                                                          \
  };                                                                           \
  template <typename T>                                                        \
  constexpr const char* NAME##Value<T>::name;                                  \
  template <class A, class B>                                                  \
  constexpr bool operator==(const NAME##Value<A>& a,                           \
                            const NAME##Value<B>& b) {                         \
    return NAME::Get(a) == NAME::Get(b);                                       \
  }                                                                            \
  template <class T>                                                           \
  std::ostream& operator<<(std::ostream& out, const NAME##Value<T>& value) {   \
    return out << #NAME "(" << value.Get() << ")";                             \
  }

    namespace __attrs_impl {

      template <class... T>
      class AttrsParent : public T... {
        protected:
        struct args_tag {};
        struct copy_tag {};
        struct move_tag {};

        template <class U>
        struct __attrs_impl_constructor_detector {
          private:
          template <class... X>
          static copy_tag Detect(const Attrs<X...>&) {
            return {};
          }

          template <class... X>
          static move_tag Detect(Attrs<X...>&&) {
            return {};
          }

          static args_tag Detect(...) { return {}; }

          public:
          using type = decltype(Detect(std::declval<U>()));
        };

        template <class U>
        using constructor_detector =
          typename __attrs_impl_constructor_detector<U>::type;

        constexpr AttrsParent() : T{}... {}

        template <class... U>
        constexpr AttrsParent(const args_tag&, U&&... args)
          : T{std::forward<U>(args)}... {}

        template <class... U>
        constexpr AttrsParent(const copy_tag&, const Attrs<U...>& other)
          : T{T::attribute_t::Get(other)}... {}

        template <class... U>
        constexpr AttrsParent(const move_tag&, Attrs<U...>&& other)
          : T{T::attribute_t::Get(std::move(other))}... {}
      };

      // This struct exists so that we can pass around Attrs<U...> without it
      // taking up any memory or having any side effects from construction.
      template <class>
      struct wrapper {};

    }  // namespace __attrs_impl

    template <typename... T>
    constexpr Attrs<std::decay_t<T>...> MakeAttrs(T&&... props);

    namespace __impl_attrs_reduce {
      template <typename>
      struct AttrsIter;

      template <>
      struct AttrsIter<__impl_variadics_type_traits::pack<>> {
        template <typename I, typename R, typename... A>
        static constexpr I&& MergeReduce(I&& init, R&& reducer, A&&... attrs) {
          return std::forward<I>(init);
        }
        template <typename I, typename R, typename... A>
        static constexpr I&& Reduce(I&& init, R&& reducer, A&&... attrs) {
          return std::forward<I>(init);
        }

        template <typename F, typename... A>
        static constexpr void MergeForeach(F&& callback, A&&... attrs) {}

        template <typename F, typename... A>
        static constexpr void Foreach(F&& callback, A&&... attrs) {}
      };

      template <typename U>
      struct AttrsIter<__impl_variadics_type_traits::pack<U>> {
        template <typename I, typename R, typename... A>
        static constexpr auto MergeReduce(I&& init, R&& reducer, A&&... attrs) {
          return std::forward<R>(reducer)(
            std::forward<I>(init), U::attribute_t::GetAttributeOrGetAttributeIn(
                                     std::forward<A>(attrs)...));
        }
        template <typename I, typename R, typename... A>
        static constexpr auto Reduce(I&& init, R&& reducer, A&&... attrs) {
          return std::forward<R>(reducer)(
            std::forward<I>(init),
            U::attribute_t::GetAttribute(std::forward<A>(attrs))...);
        }
        template <typename F, typename... A>
        static constexpr void MergeForeach(F&& callback, A&&... attrs) {
          std::forward<F>(callback)(
            U::attribute_t::GetAttributeOrGetAttributeIn(
              std::forward<A>(attrs)...));
        }
        template <typename F, typename... A>
        static constexpr void Foreach(F&& callback, A&&... attrs) {
          std::forward<F>(callback)(
            U::attribute_t::GetAttribute(std::forward<A>(attrs))...);
        }
      };

      template <typename U0, typename U1, typename... U>
      struct AttrsIter<__impl_variadics_type_traits::pack<U0, U1, U...>> {
        template <typename I, typename R, typename... A>
        static constexpr auto MergeReduce(I&& init, R&& reducer, A&&... attrs) {
          using new_init_t =
            decltype(reducer(std::forward<I>(init),
                             U0::attribute_t::GetAttributeOrGetAttributeIn(
                               std::forward<A>(attrs)...)));

          return AttrsIter<__impl_variadics_type_traits::pack<U1, U...>>::
            MergeReduce(std::forward<new_init_t>(
                          reducer(std::forward<I>(init),
                                  U0::attribute_t::GetAttributeOrGetAttributeIn(
                                    std::forward<A>(attrs)...))),
                        std::forward<R>(reducer), std::forward<A>(attrs)...);
        }

        template <typename I, typename R, typename... A>
        static constexpr auto Reduce(I&& init, R&& reducer, A&&... attrs) {
          using new_init_t = decltype(
            reducer(std::forward<I>(init),
                    U0::attribute_t::GetAttribute(std::forward<A>(attrs))...));

          return AttrsIter<__impl_variadics_type_traits::pack<U1, U...>>::
            Reduce(std::forward<new_init_t>(reducer(
                     std::forward<I>(init),
                     U0::attribute_t::GetAttribute(std::forward<A>(attrs))...)),
                   std::forward<R>(reducer), std::forward<A>(attrs)...);
        }

        template <typename F, typename... A>
        static constexpr void MergeForeach(F&& callback, A&&... attrs) {
          callback(U0::attribute_t::GetAttributeOrGetAttributeIn(
            std::forward<A>(attrs)...));

          return AttrsIter<__impl_variadics_type_traits::pack<U1, U...>>::
            MergeForeach(std::forward<F>(callback), std::forward<A>(attrs)...);
        }

        template <typename F, typename... A>
        static constexpr void Foreach(F&& callback, A&&... attrs) {
          callback(U0::attribute_t::GetAttribute(std::forward<A>(attrs))...);

          return AttrsIter<__impl_variadics_type_traits::pack<U1, U...>>::
            Foreach(std::forward<F>(callback), std::forward<A>(attrs)...);
        }
      };

    }  // namespace __impl_attrs_reduce

    template <typename I, typename R, typename... A>
    constexpr auto MergeReduce(I&& init, R&& reducer, A&&... attrs) {
      return __impl_attrs_reduce::AttrsIter<
        variadic_union_t<is_same_attribute,
                         __impl_variadics_type_traits::ToPackType<
                           Attrs, typename std::decay_t<A>::attributes_t>...>>::
        MergeReduce(std::forward<I>(init), std::forward<R>(reducer),
                    std::forward<A>(attrs)...);
    }

    template <typename I, typename R, typename... A>
    constexpr auto Reduce(I&& init, R&& reducer, A&&... attrs) {
      return __impl_attrs_reduce::AttrsIter<variadic_intersection_t<
        is_same_attribute,
        __impl_variadics_type_traits::ToPackType<
          Attrs, typename std::decay_t<A>::attributes_t>...>>::
        Reduce(std::forward<I>(init), std::forward<R>(reducer),
               std::forward<A>(attrs)...);
    }

    template <typename F, typename... A>
    constexpr void MergeForeach(F&& callback, A&&... attrs) {
      return __impl_attrs_reduce::AttrsIter<
        variadic_union_t<is_same_attribute,
                         __impl_variadics_type_traits::ToPackType<
                           Attrs, typename std::decay_t<A>::attributes_t>...>>::
        MergeForeach(std::forward<F>(callback), std::forward<A>(attrs)...);
    }
    template <typename F, typename... A>
    constexpr void Foreach(F&& callback, A&&... attrs) {
      return __impl_attrs_reduce::AttrsIter<variadic_intersection_t<
        is_same_attribute,
        __impl_variadics_type_traits::ToPackType<
          Attrs, typename std::decay_t<A>::attributes_t>...>>::
        Foreach(std::forward<F>(callback), std::forward<A>(attrs)...);
    }

    template <typename... T>
    class Attrs : public __attrs_impl::AttrsParent<T...> {
      public:
      using attributes_t = Attrs;
      using pack_t = TypePack<T...>;

      constexpr Attrs() : __attrs_impl::AttrsParent<T...>{} {}

      // This is one of the really nasty parts. The problem is that we really
      // want two constructors
      //
      // (1) template<class... U> Attrs(U&& ... MakeAttrs),
      // which initializes all the members of the attribute pack and
      //
      // (2) template<class... U> Attrs(const Attrs<U...>& other),
      // which copies the data out of the other attribute pack and into this
      // one.
      //
      // Sadly, C++ can't tell the difference between these two signatures,
      // and will only Call the first because of how c++ decides the order to
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
      constexpr static struct __impl_AssignOp_asssigner_t {
        template <typename T1, typename T2>
        constexpr void operator()(T1& to, T2&& from) const {
          to = std::forward<T2>(from);
        }
      } __impl_AssignOp_asssigner{};

      public:
      template <typename F>
      Attrs& operator=(F&& from) {
        emp::tools::Foreach(__impl_AssignOp_asssigner, *this,
                            std ::forward<F>(from));

        return *this;
      }

      private:
      template <bool Last, typename A>
      struct GetAttr;
      template <typename A>
      struct GetAttr<false, A> {
        template <typename S, typename... U>
        static constexpr decltype(auto) Get(S&& Self, U&&... args) {
          using attribute_t = typename A::attribute_t;
          return attribute_t::CallOrGetAttribute(std::forward<S>(Self),
                                                 args...);
        }
      };

      template <typename A>
      struct GetAttr<true, A> {
        template <typename S, typename... U>
        static constexpr decltype(auto) Get(S&& Self, U&&... args) {
          using attribute_t = typename A::attribute_t;
          return attribute_t::CallOrGetAttribute(std::forward<S>(Self),
                                                 std::forward<U>(args)...);
        }
      };

      template <typename S, typename... U>
      static constexpr auto Call(S&& Self, U&&... args) {
        constexpr size_t last = sizeof...(T) - 1;
        return MakeAttrs(
          GetAttr<variadic_index_of_v<T, Attrs> == last,
                  T>::template Get<S, U...>(std::forward<S>(Self),
                                            std::forward<U>(args)...)...);
      }

      public:
      /// Provide a call operator for attribute packs. This will attempt to call
      /// each member of the attribute pack with the given arguments, and maps
      /// the result of that call into a new parameter pack as the same
      /// attribute. If one of the members of the pack cannot be called with the
      /// given pack, then it is put into the new pack unchanged. Mostly, this
      /// is meant to allow the opengl plotting library to do mapping operations
      template <class... U>
      constexpr auto operator()(U&&... args) & {
        return Call(*this, std::forward<U>(args)...);
      }

      template <class... U>
      constexpr auto operator()(U&&... args) const & {
        return Call(*this, std::forward<U>(args)...);
      }

      template <class... U>
      constexpr auto operator()(U&&... args) && {
        return Call(std::move(*this), std::forward<U>(args)...);
      }

      template <class... U>
      constexpr auto operator()(U&&... args) const && {
        return Call(std::move(*this), std::forward<U>(args)...);
      }

      private:
      struct eq_reducer {
        template <typename A, typename B>
        constexpr bool operator()(bool init, A&& a, B&& b) const {
          return init && (std::forward<A>(a).Get() == std::forward<B>(b).Get());
        }
      };

      public:
      /// Attribute packs support equals and not equals, but are not
      /// orderable, as the members are not considered to be ordered from the
      /// user's perspective.
      template <class... U>
      constexpr bool operator==(const Attrs<U...>& other) const {
        // @todo: this needs more carefull checking, as it will have strange
        // behavior when the other attribute pack has different types
        return tools::Reduce(true, eq_reducer{}, *this, other);
      }

      private:
      template <typename F>
      struct __impl_reduce_from_reduce_value {
        F callback;
        constexpr __impl_reduce_from_reduce_value(F callback)
          : callback(std::forward<F>(callback)) {}
        template <typename I, typename U>
        constexpr auto operator()(I&& init, U&& value) {
          return std::forward<F>(callback)(value.name, std::forward<I>(init),
                                           *std::forward<U>(value));
        }
      };

      public:
      template <typename I, typename F>
      constexpr auto AttributeReduce(I&& init, F&& callback) & {
        return emp::tools::Reduce(std::forward<I>(init),
                                  std::forward<F>(callback), *this);
      }
      template <typename I, typename F>
      constexpr auto AttributeReduce(I&& init, F&& callback) const & {
        return emp::tools::Reduce(std::forward<I>(init),
                                  std::forward<F>(callback), *this);
      }
      template <typename I, typename F>
      constexpr auto AttributeReduce(I&& init, F&& callback) && {
        return emp::tools::Reduce(std::forward<I>(init),
                                  std::forward<F>(callback), std::move(*this));
      }
      template <typename I, typename F>
      constexpr auto AttributeReduce(I&& init, F&& callback) const && {
        return emp::tools::Reduce(std::forward<I>(init),
                                  std::forward<F>(callback), std::move(*this));
      }

      template <typename I, typename F>
      constexpr auto Reduce(I&& init, F&& callback) {
        return AttributeReduce(
          std::forward<I>(init),
          __impl_reduce_from_reduce_value<F&&>{std::forward<F>(callback)});
      }
      template <typename I, typename F>
      constexpr auto Reduce(I&& init, F&& callback) const {
        return AttributeReduce(
          std::forward<I>(init),
          __impl_reduce_from_reduce_value<F&&>{std::forward<F>(callback)});
      }

      private:
      template <typename F>
      struct __impl_foreach_from_foreach_value {
        F callback;
        constexpr __impl_foreach_from_foreach_value(F callback)
          : callback(std::forward<F>(callback)) {}
        template <typename U>
        constexpr void operator()(U&& value) {
          std::forward<F>(callback)(value.name, *std::forward<U>(value));
        }
      };

      public:
      template <typename F>
      constexpr void AttributeForeach(F&& callback) & {
        tools::Foreach(std::forward<F>(callback), *this);
      }

      template <typename F>
      constexpr void AttributeForeach(F&& callback) const & {
        tools::Foreach(std::forward<F>(callback), *this);
      }

      template <typename F>
      constexpr void AttributeForeach(F&& callback) && {
        tools::Foreach(std::forward<F>(callback), std::move(*this));
      }

      template <typename F>
      constexpr void AttributeForeach(F&& callback) const && {
        tools::Foreach(std::forward<F>(callback), std::move(*this));
      }

      template <typename F>
      constexpr void Foreach(F&& callback) {
        AttributeForeach(
          __impl_foreach_from_foreach_value<decltype(
            std::forward<F>(callback))>{std::forward<F>(callback)});
      }

      template <typename F>
      constexpr void Foreach(F&& callback) const {
        AttributeForeach(
          __impl_foreach_from_foreach_value<decltype(
            std::forward<F>(callback))>{std::forward<F>(callback)});
      }

      private:
      template <typename S, typename U>
      constexpr static Attrs<T..., std::decay_t<U>> __impl_SetAttribute(
        const std::false_type& has_attr, S&& self, U&& attribute) {
        return MakeAttrs(T::attribute_t::GetAttribute(std::forward<S>(self))...,
                         std::forward<U>(attribute));
      }
      template <typename S, typename U>
      constexpr static auto __impl_SetAttribute(const std::true_type& has_attr,
                                                S&& self, U&& attribute) {
        return MakeAttrs(T::attribute_t::GetAttributeOrGetAttributeIn(
          std::forward<S>(self), std::forward<U>(attribute))...);
      }

      template <typename S, typename U>
      constexpr static auto __impl_SetAttribute(S&& self, U&& attribute) {
        return __impl_SetAttribute(
          has_attribute<S, typename std::decay_t<U>::attribute_t>{},
          std::forward<S>(self), std::forward<U>(attribute));
      }

      public:
      template <typename U>
      constexpr auto SetAttribute(U&& attribute) & {
        return __impl_SetAttribute(*this, std::forward<U>(attribute));
      }

      template <typename U>
      constexpr auto SetAttribute(U&& attribute) const & {
        return __impl_SetAttribute(*this, std::forward<U>(attribute));
      }

      template <typename U>
      constexpr auto SetAttribute(U&& attribute) && {
        return __impl_SetAttribute(std::move(*this),
                                   std::forward<U>(attribute));
      }

      template <typename U>
      constexpr auto SetAttribute(U&& attribute) const && {
        return __impl_SetAttribute(std::move(*this),
                                   std::forward<U>(attribute));
      }
    };

    template <typename... T>
    constexpr typename Attrs<T...>::__impl_AssignOp_asssigner_t
      Attrs<T...>::__impl_AssignOp_asssigner;

    ///  An alternative syntax for creating attribute packs. Takes any number
    ///  of attributes and returns a pack containing each of those attributes.
    template <class... T>
    constexpr Attrs<std::decay_t<T>...> MakeAttrs(T&&... props) {
      return {std::forward<T>(props)...};
    }

    namespace __impl_attrs_merge {
      constexpr struct {
        template <typename I, typename A>
        constexpr auto operator()(I&& init, A&& next) const {
          return std::forward<I>(init).SetAttribute(std::forward<A>(next));
        }
      } attrs_merge{};
    }  // namespace __impl_attrs_merge

    /// Creates a new attribute pack which has all the attributes of this
    /// pack and another pack. Values will be taken from other other pack
    /// preferentially.
    #ifndef DOXYGEN_SHOULD_SKIP_THIS // Doxygen is getting tripped up by this
    template <typename... U>
    constexpr auto Merge(U&&... packs)
      // This hint is required by some older compilers
      -> decltype(MergeReduce(Attrs<>{}, __impl_attrs_merge::attrs_merge,
                              std::forward<U>(packs)...)) {
      return MergeReduce(Attrs<>{}, __impl_attrs_merge::attrs_merge,
                         std::forward<U>(packs)...);
    }
    #else
    template <typename... U>
    constexpr auto Merge(U&&... packs) {;}
    #endif /*DOXYGEN_SHOULD_SKIP_THIS*/

    namespace __attrs_impl {
      template <class...>
      struct print_attrs_tag {};

      template <class... T, class H>
      void PrintAttrs(std::ostream& out, const Attrs<T...>& attrs,
                      const print_attrs_tag<H>&) {
        out << '"' << H::name << "\": " << H::attribute_t::Get(attrs);
      }
      template <class... T, class H0, class H1, class... U>
      void PrintAttrs(std::ostream& out, const Attrs<T...>& attrs,
                      const print_attrs_tag<H0, H1, U...>&) {
        out << '"' << H0::name << "\": " << H0::attribute_t::Get(attrs) << ", ";
        PrintAttrs(out, attrs, print_attrs_tag<H1, U...>{});
      }
    }  // namespace __attrs_impl

    template <class H, class... T>
    std::ostream& operator<<(std::ostream& out, const Attrs<H, T...>& attrs) {
      out << "{ ";
      __attrs_impl::PrintAttrs(out, attrs,
                               __attrs_impl::print_attrs_tag<H, T...>{});
      return out << " }";
    }

    std::ostream& operator<<(std::ostream& out, const Attrs<>& attrs) {
      return out << "{ }";
    }
  }  // namespace tools
}  // namespace emp

#endif  // EMP_PLOT_EMP
