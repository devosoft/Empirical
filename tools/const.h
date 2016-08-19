//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Commonly used values...

#ifndef EMP_CONST_H
#define EMP_CONST_H

namespace emp {

  constexpr const double E      = 2.71828;                // e
  constexpr const double PHI    = 1.61803398874;          // Golden ratio
  constexpr const double PI     = 3.14159265358979;       // pi
  constexpr const double SQRT2  = 1.41421356237310;       // sqrt(2)

  constexpr const uint32_t MAX_BYTE  = 255;           // (2^8 - 1)
  constexpr const uint32_t MAX_2BYTE = 65535;         // (2^16 - 1)
  constexpr const uint32_t MAX_WORD  = 65535;         // (2^16 - 1)
  constexpr const uint32_t MAX_3BYTE = 16777215;      // (2^24 - 1)
  constexpr const uint32_t MAX_UINT  = 4294967295;    // (2^32 - 1)
  constexpr const uint32_t MAX_4BYTE = 4294967295;    // (2^32 - 1)

  constexpr const int32_t MIN_INT = -2147483648;      // (- 2^31)

}

#endif
