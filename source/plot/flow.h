#ifndef EMP_PLOT_FLOW_H
#define EMP_PLOT_FLOW_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

#include "tools/attrs.h"

namespace emp {
  namespace plot {
    struct flow_member_tag {};
    template <class T>
    using is_flow_member = std::is_base_of<flow_member_tag, T>;
    template <class T>
    constexpr auto is_flow_member_v = is_flow_member<T>::value;

    template <class D>
    struct Joinable;

    template <class From, class To>
    class Join : public Joinable<Join<From, To>> {
      From from;
      To to;
      template <class T, class... Args>
      static decltype(auto) ApplyTo(std::shared_ptr<T> target, Args&&... args) {
        return target->Apply(std::forward<Args>(args)...);
      }

      template <class T, class... Args>
      static decltype(auto) ApplyTo(const T& target, Args&&... args) {
        return target.Apply(std::forward<Args>(args)...);
      }

      template <class DataIter, class PrevIter>
      auto _ApplySecond(const std::true_type& toIsVoid, DataIter dbegin,
                        DataIter dend, PrevIter pbegin, PrevIter pend) const {
        ApplyTo(to, dbegin, dend, pbegin, pend);
        return std::vector<typename std::iterator_traits<PrevIter>::value_type>(
          pbegin, pend);
      }

      template <class... Args>
      decltype(auto) _ApplySecond(const std::false_type& toIsVoid,
                                  Args&&... args) const {
        return ApplyTo(to, std::forward<Args>(args)...);
      }

      template <class... Args>
      decltype(auto) ApplySecond(Args&&... args) const {
        return _ApplySecond(
          std::is_void<decltype(ApplyTo(to, std::forward<Args>(args)...))>{},
          std::forward<Args>(args)...);
      }

      template <class Iter, class... Args>
      decltype(auto) Apply(const std::false_type& fromIsVoid, Iter begin,
                           Iter end, Args&&... args) const {
        auto intermediate{
          ApplyTo(from, begin, end, std::forward<Args>(args)...)};
        return ApplySecond(begin, end, std::begin(intermediate),
                           std::end(intermediate));
      }

      // template <class... Args>
      // decltype(auto) Apply(const std::true_type& fromIsVoid,
      //                      Args&&... args) const {
      //   ApplyTo(from, args...);
      //   return ApplySecond(std::forward<Args>(args)...);
      // }

      public:
      template <class _From, class _To>
      Join(_From&& from, _To&& to)
        : from(std::forward<_From>(from)), to(std::forward<_To>(to)) {}

      template <class DataIter, class PrevIter>
      decltype(auto) Apply(DataIter dbegin, DataIter dend, PrevIter pbegin,
                           PrevIter pend) const {
        return Apply(
          std::is_void<decltype(ApplyTo(from, dbegin, dend, pbegin, pend))>{},
          dbegin, dend, pbegin, pend);
      }

      template <class DataIter>
      decltype(auto) Apply(DataIter begin, DataIter end) const {
        return Apply(std::is_void<decltype(ApplyTo(from, begin, end))>{}, begin,
                     end);
      }
    };

    template <class D>
    struct Joinable {
      constexpr Joinable() = default;
      constexpr Joinable(const Joinable&) = default;
      constexpr Joinable(Joinable&&) = default;

      template <class T>
      constexpr Join<D, std::decay_t<T>> Join(T&& to) const {
        return {static_cast<const D&>(*this), std::forward<T>(to)};
      }
    };

    template <class From, class To>
    auto operator>>(const Joinable<From>& from, To&& to) {
      return from.Join(std::forward<To>(to));
    }

    template <class From, class To>
    auto operator>>(Joinable<From>&& from, To&& to) {
      return std::move(from).Join(std::forward<To>(to));
    }

    template <typename... A>
    class AttrsMap : public Joinable<AttrsMap<A...>> {
      tools::Attrs<A...> attrs;

      public:
      AttrsMap(tools::Attrs<A...>& attrs) : attrs(attrs) {}
      AttrsMap(const tools::Attrs<A...>& attrs) : attrs(attrs) {}
      AttrsMap(tools::Attrs<A...>&& attrs) : attrs(std::move(attrs)) {}
      AttrsMap(const tools::Attrs<A...>&& attrs) : attrs(std::move(attrs)) {}

      template <class DataIter, class PropsIter>
      auto Apply(DataIter dbegin, DataIter dend, PropsIter pbegin,
                 PropsIter pend) const {
        using result_t = decltype(Merge(attrs(*dbegin), *pbegin));
        std::vector<result_t> results;
        while (dbegin != dend && pbegin != pend) {
          results.emplace_back(Merge(attrs(*dbegin), *pbegin));
          ++dbegin;
          ++pbegin;
        }

        return results;
      }

      template <class DataIter>
      auto Apply(DataIter dbegin, DataIter dend) const {
        using result_t = decltype(attrs(*dbegin));
        std::vector<result_t> results;
        while (dbegin != dend) {
          results.emplace_back(attrs(*dbegin));
          ++dbegin;
        }
        return results;
      }
    };

    template <typename... A, class To>
    auto operator>>(tools::Attrs<A...>& from, To&& to) {
      return AttrsMap<std::decay_t<A>...>{from} >> std::forward<To>(to);
    }
    template <typename... A, class To>
    auto operator>>(const tools::Attrs<A...>& from, To&& to) {
      return AttrsMap<std::decay_t<A>...>{from} >> std::forward<To>(to);
    }
    template <typename... A, class To>
    auto operator>>(tools::Attrs<A...>&& from, To&& to) {
      return AttrsMap<std::decay_t<A>...>{std::move(from)} >>
             std::forward<To>(to);
    }
    template <typename... A, class To>
    auto operator>>(const tools::Attrs<A...>&& from, To&& to) {
      return AttrsMap<std::decay_t<A>...>{std::move(from)} >>
             std::forward<To>(to);
    }

  }  // namespace plot

}  // namespace emp

#endif  // EMP_PLOT_FLOW_H
