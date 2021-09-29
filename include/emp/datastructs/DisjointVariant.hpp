/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  DisjointVariant.hpp
 *  @brief A container similar to std::variant, where only one of a set of
 *  types can be active, but state is maintained for inactive types (they are
 *  not destructed or overwritten).
 */


#ifndef EMP_DISJOINT_VARIANT_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../polyfill/type_identity.hpp"

namespace emp {

template<typename ...Ts>
class DisjointVariant {

  // Holds state for each element.
  std::tuple<Ts...> disjoint_data;

  /// Tracks which type is active.
  std::variant<std::type_identity<Ts>...> active_typeid;

  public:

  /// Forwarding constructor.
  template<typename... Args>
  DisjointVariant(Args&&... args)
  : disjoint_data(std::forward<Args>(args)...)
  {}

  /// Switch which data element is active.
  template<typename T>
  void Activate() {
    using wrapped_active_type_t = std::type_identity<T>;
    active_typeid.template emplace<wrapped_active_type_t>();
  }

  /// Assign to data element.
  template<typename T>
  void AssignToElement(T&& val) {
    std::get<T>(disjoint_data) = std::forward<T>(val);
  }

  /// Assign data element and set that element as active.
  template<typename T>
  void AssignAndActivate(T&& val) {
    AssignToElement<T>( std::forward<T>(val) );
    Activate<T>();
  }

  /// Wraps std::visit to execute visitor on active data element.
  template<class Visitor>
  decltype(auto) Visit(Visitor&& visitor) {
    return std::visit(
      [this, &visitor]( const auto& typeid_ ){
        using wrapped_active_type_t = std::decay_t<decltype(typeid_)>;
        using active_type_t = typename wrapped_active_type_t::type;
        auto& active_data = std::get<active_type_t>( disjoint_data );
        return std::forward<Visitor>(visitor)(active_data);
      },
      active_typeid
    );
  }

};

} // namespace emp

#endif
