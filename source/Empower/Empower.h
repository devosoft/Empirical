/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Empower.h
 *  @brief A scripting language built inside of C++
 *
 *  Empower is a scripting language built inside of Empirical to simplify and the use of fast
 *  run-time interpreting.  Internally, and Empower object will track all of the types used and 
 *  all of the variables declared, ensuring that they interact correctly.
 * 
 */

#ifndef EMP_EMPOWER_H
#define EMP_EMPOWER_H

#include "../base/vector.h"

namespace emp {

  class Empower {
  public:
    using byte_t = unsigned char;
    using memory_t = emp::vector<byte_t>;

  protected:
    memory_t memory;

  public:
    Empower() : memory() { ; }
    ~Empower() { ; }

    
  };

}

#endif
