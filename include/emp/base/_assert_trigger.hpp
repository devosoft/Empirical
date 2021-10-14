/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _assert_trigger.hpp
 *  @brief Assert trigger implementation selector.
 *  @note For internal use.
 */

#ifndef EMP_BASE__ASSERT_TRIGGER_HPP_INCLUDE
#define EMP_BASE__ASSERT_TRIGGER_HPP_INCLUDE

/// TDEBUG hould trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif

#if defined( __EMSCRIPTEN__ )
  #include "_emscripten_assert_trigger.hpp"
#elif defined( EMP_TDEBUG )
  #include "_tdebug_assert_trigger.hpp"
#else
  #include "_native_assert_trigger.hpp"
#endif

#endif // #ifndef EMP_BASE__ASSERT_TRIGGER_HPP_INCLUDE
