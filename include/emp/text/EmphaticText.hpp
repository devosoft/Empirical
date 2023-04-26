/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2023.
 *
 *  @file EmphaticText.hpp
 *  @brief Like emp::Text, but defaults to using EmphaticEncoding.
 *  @note Status: ALPHA
 * 
 */

#ifndef EMP_TOOLS_EMPHATIC_TEXT_HPP_INCLUDE
#define EMP_TOOLS_EMPHATIC_TEXT_HPP_INCLUDE

#include "Text.hpp"
#include "EmphaticEncoding.hpp"

namespace emp {

  class EmphaticText : public emp::Text {
  public:
    /// @brief Create a new, default EmphaticText object.
    EmphaticText() { AddEncoding<EmphaticEncoding>(); }

    /// @brief Copy over another Text object, but make it use an Emphatic encoding.
    /// @param in Starting text to load in.
    EmphaticText(const Text & in) : Text(in) { ActivateEncoding<EmphaticEncoding>("emphatic"); }

    /// @brief Create a new EmphaticText object and default it to the provided string.
    /// @param in Starting (emphatic-encoded) string to use.
    EmphaticText(const String & in)      { AppendAs<EmphaticEncoding>("emphatic", in); }
    EmphaticText(const std::string & in) { AppendAs<EmphaticEncoding>("emphatic", in); }
    EmphaticText(const char * in)        { AppendAs<EmphaticEncoding>("emphatic", in); }

    EmphaticText & operator=(const EmphaticText &) = default;
    EmphaticText & operator=(EmphaticText &&) = default;
    template <typename T>
    EmphaticText & operator=(T && in) { Text::operator=(std::forward<T>(in)); return *this; };
  };

}


#endif