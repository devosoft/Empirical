// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_CONST_H
#define EMP_CONST_H

namespace emp {
  const double E      = 2.71828;                // e
  const double PHI    = 1.61803398874;          // Golden ratio
  const double PI     = 3.14159265358979;       // pi
  const double SQRT2  = 1.41421356237310;       // sqrt(2)

  const uint32_t MAX_BYTE  = 255;           // (2^8 - 1)
  const uint32_t MAX_2BYTE = 65535;         // (2^16 - 1)
  const uint32_t MAX_WORD  = 65535;         // (2^16 - 1)
  const uint32_t MAX_3BYTE = 16777215;      // (2^24 - 1)
  const uint32_t MAX_UINT  = 4294967295;    // (2^32 - 1)
  const uint32_t MAX_4BYTE = 4294967295;    // (2^32 - 1)
};

#endif
