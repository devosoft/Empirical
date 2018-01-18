/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Trait.h
 *  @brief Directly measure a target quality about a type of object.
 *
 *  These objects are able to measure a specific trait on another object.  They
 *  (will eventually) interface smoothly with DataNodes for data collection.
 */

#ifndef EMP_DATA_TRAIT_H
#define EMP_DATA_TRAIT_H

#include "../base/assert.h"
#include "../tools/Range.h"

namespace emp {

  template <typename TARGET_T, typename VALUE_T=double>
  class Trait {
  public:
    using target_t = TARGET_T;
    using value_t = VALUE_T;
  protected:
    std::string name;                        ///< Unique name for this trait.
    std::string desc;                        ///< Full description explaining this trait.
    std::function<value_t(target_t &)> fun;   ///< Function to retrieve trait value from target.
    emp::Range<value_t> range;                ///< Limits on legal values (defaults to extremes)

  public:
    Trait(const std::string & _n, const std::function<double(TARGET_T&)> & _f)
      : name(_n), fun(_f), range() { ; }
    Trait(const std::string & _n, const std::function<double(TARGET_T&)> & _f,
          value_t min, value_t max)
      : name(_n), fun(_f), range(min, max) { ; }

  };


}

#endif
