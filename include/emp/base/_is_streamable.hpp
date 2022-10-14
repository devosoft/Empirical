/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _is_streamable.hpp
 *  @brief Test at compile time whether a type can be streamed.
 *  @note This header is for internal use to preserve levelization.
 *  Include meta/type_traits.hpp to use is_streamable.
 */

#ifndef EMP_IS_STREAMABLE_HPP
#define EMP_IS_STREAMABLE_HPP

#include <type_traits>
#include <utility>

namespace emp {

// adapted from https://stackoverflow.com/a/22759544
template<typename S, typename T>
class is_streamable {

  template<typename SS, typename TT>
  static auto test(int) -> decltype(
    std::declval<SS&>() << std::declval<TT>(), std::true_type()
  );

  template<typename, typename>
  static auto test(...) -> std::false_type;

public:

  /// Determine if a type can be streamed.
  static constexpr bool value = decltype(test<S,T>(0))::value;

};

} // namespace emp

#endif // #ifndef EMP_IS_STREAMABLE_HPP
