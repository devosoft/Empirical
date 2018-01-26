#ifndef EMP_PLOT_FLOW_H
#define EMP_PLOT_FLOW_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

#include "math/region.h"

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
      static decltype(auto) applyTo(std::shared_ptr<T> target, Args&&... args) {
        return target->apply(std::forward<Args>(args)...);
      }

      template <class T, class... Args>
      static decltype(auto) applyTo(const T& target, Args&&... args) {
        return target.apply(std::forward<Args>(args)...);
      }

      template <class DataIter, class PrevIter>
      auto _applySecond(const std::true_type& toIsVoid, DataIter dbegin,
                        DataIter dend, PrevIter pbegin, PrevIter pend) const {
        applyTo(to, dbegin, dend, pbegin, pend);
        return std::vector<typename std::iterator_traits<PrevIter>::value_type>(
          pbegin, pend);
      }

      template <class... Args>
      decltype(auto) _applySecond(const std::false_type& toIsVoid,
                                  Args&&... args) const {
        return applyTo(to, std::forward<Args>(args)...);
      }

      template <class... Args>
      decltype(auto) applySecond(Args&&... args) const {
        return _applySecond(
          std::is_void<decltype(applyTo(to, std::forward<Args>(args)...))>{},
          std::forward<Args>(args)...);
      }

      template <class Iter, class... Args>
      decltype(auto) apply(const std::false_type& fromIsVoid, Iter begin,
                           Iter end, Args&&... args) const {
        auto intermediate{
          applyTo(from, begin, end, std::forward<Args>(args)...)};
        return applySecond(begin, end, std::begin(intermediate),
                           std::end(intermediate));
      }

      template <class... Args>
      decltype(auto) apply(const std::true_type& fromIsVoid,
                           Args&&... args) const {
        applyTo(from, args...);
        return applySecond(std::forward<Args>(args)...);
      }

      public:
      template <class _From, class _To>
      Join(_From&& from, _To&& to)
        : from(std::forward<_From>(from)), to(std::forward<_To>(to)) {}

      template <class DataIter, class PrevIter>
      decltype(auto) apply(DataIter dbegin, DataIter dend, PrevIter pbegin,
                           PrevIter pend) const {
        return apply(
          std::is_void<decltype(applyTo(from, dbegin, dend, pbegin, pend))>{},
          dbegin, dend, pbegin, pend);
      }

      template <class DataIter>
      decltype(auto) apply(DataIter begin, DataIter end) const {
        return apply(std::is_void<decltype(applyTo(from, begin, end))>{}, begin,
                     end);
      }
    };

    template <class D>
    struct Joinable {
      template <class T>
      constexpr Join<D, std::decay_t<T>> join(T&& to) const {
        return {static_cast<const D&>(*this), std::forward<T>(to)};
      }
    };

    template <class From, class To>
    auto operator>>(const Joinable<From>& from, To&& to) {
      return from.join(std::forward<To>(to));
    }

  }  // namespace plot

}  // namespace emp

#endif  // EMP_PLOT_FLOW_H
