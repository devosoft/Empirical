/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file _optional_throw.hpp
 *  @brief Variant of asserts that throws exception
 *  @note This is useful for writing code that gets wrapped in Python via pybind11
 */

#ifndef EMP_BASE__OPTIONAL_THROW_TRIGGER_HPP_INCLUDE
#define EMP_BASE__OPTIONAL_THROW_TRIGGER_HPP_INCLUDE

#include <sstream>
#include <string>

#include "_is_streamable.hpp"

namespace emp {

    /// Base case for assert_print...
    inline void assert_print(std::stringstream &) { ; }

    /// Print out information about the next variable and recurse...
    template <typename T, typename... EXTRA>
    void assert_print(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
        if constexpr ( emp::is_streamable<std::stringstream, T>::value ) {
        ss << name << ": [" << val << "]" << std::endl;
        } else ss << name << ": (non-streamable type)" << std::endl;
        assert_print(ss, std::forward<EXTRA>(extra)...);
    }

    template <typename... EXTRA>
    void assert_throw(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
        std::stringstream ss;
        ss << "Internal Error (in " << filename << " line " << line << "): " << expr << '\n';
        assert_print(ss, std::forward<EXTRA>(extra)...);
        throw(ss.str());
    }
}

#endif // #ifndef EMP_BASE__OPTIONAL_THROW_TRIGGER_HPP_INCLUDE