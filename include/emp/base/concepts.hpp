/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023-24.
 *
 *  @file concepts.hpp
 *  @brief Useful concepts that are not trivially available in the C++20 standard library.
 * 
 *  The concepts available here are:
 * 
 *    canStreamTo<STREAM_T, OBJECT_T> 
 *    canStreamFrom<STREAM_T, OBJECT_T> 
 * 
 */

#ifndef EMP_BASE_CONCEPTS_HPP_INCLUDE
#define EMP_BASE_CONCEPTS_HPP_INCLUDE


#include <cstdint>       // uint8_t, uint16_t, etc.
#include <functional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

namespace emp {

  // === Predeclarations used below ===
  template <typename TYPE> class Ptr;
  #ifdef NDEBUG
  template <typename T, typename... Ts> using vector = std::vector<T, Ts...>;
  #else
  template <typename T, typename... Ts> class vector;
  #endif


  // === Concepts to determine if a type can be streamed ===

  /// Concept to identify if a type can be sent into an ostream.
  template <typename STREAM_T, typename OBJ_T>
  concept canStreamTo = requires(STREAM_T & stream, OBJ_T value) {
    { stream << value } -> std::convertible_to<std::ostream&>;
  };

  /// Concept to identify if a type can be set from an istream.
  template <typename STREAM_T, typename OBJ_T>
  concept canStreamFrom = requires(STREAM_T & stream, OBJ_T value) {
    { stream >> value } -> std::convertible_to<std::istream&>;
  };


  // ===== Member function types used throughout Empirical =====

  /// Does a type have a ToString() member function?
  template <typename OBJ_T>
  concept hasToString = requires(OBJ_T value) {
    { value.ToString(std::cout) } -> std::same_as<std::string>;
  };

  /// Does a type have a ToDouble() member function?
  template <typename OBJ_T>
  concept hasToDouble = requires(OBJ_T value) {
    { value.ToDouble(std::cout) } -> std::same_as<double>;
  };

  /// Does a type have a FromString() member function?
  template<typename T>
  concept hasFromString = requires(T t, const std::string& s, const char* c) {
      { t.FromString(s) }; // Test with std::string
      { t.FromString(c) }; // Test with const char*
  };

  /// Does a type have a FromString() member function?
  template<typename T>
  concept hasFromDouble = requires(T t, double d) {
      { t.FromDouble(d) };
  };

  // ===== Concepts to identify particular template types =====

  /// Test if a given type T is an instance of template TEMPLATE
  template <typename T, template <typename...> class TEMPLATE>
  struct is_template : std::false_type {};

  template <template <typename...> class TEMPLATE, typename... ARG_Ts>
  struct is_template<TEMPLATE<ARG_Ts...>, TEMPLATE> : std::true_type {};

  /// Test if a given type T is an instance of template TEMPLATE with one type and one non-type arg
  template <typename T, template <typename, auto> class TEMPLATE>
  struct is_template_tn : std::false_type {};

  template <template <typename, auto> class TEMPLATE, typename ARG1, auto ARG2>
  struct is_template_tn<TEMPLATE<ARG1, ARG2>, TEMPLATE> : std::true_type {};


  template <typename T> using is_std_function = is_template<T, std::function>;
  template <typename T> using is_emp_vector = is_template<T, emp::vector>;
  template <typename T> using is_span = is_template_tn<T, std::span>;


  // ===== For backward compatability only =====

  /// A type trait to identify if a type is streamable (for backward compatability)
  template <typename T>
  struct is_streamable : std::bool_constant<canStreamTo<std::ostream, T>> {};

  /// A type trait to identify if a type is iterable.
  template <typename T>
  struct IsIterable : std::bool_constant<std::ranges::range<T>> {};

}  // namespace emp

#endif // #ifndef EMP_META_TYPE_TRAITS_HPP_INCLUDE
