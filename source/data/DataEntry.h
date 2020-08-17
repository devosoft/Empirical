/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  DataEntry.h
 *  @brief A single piece of data with an associated type.
 *  @note Status: ALPHA
 *
 *  A DataEntry pairs a memory position with a TypeID for dynamic variable manipulation.
 *
 */

#ifndef EMP_DATA_ENTRY_H
#define EMP_DATA_ENTRY_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../meta/TypeID.h"

namespace emp {

  class DataEntry {
  protected:
    emp::Ptr<void> data = nullptr;
    emp::TypeID type;

    DataEntry() { }
    DataEntry(const DataEntry &) = default;

    DataEntry & operator=(const DataEntry &) = default;

    double ToDouble() { return type.ToDouble(data); }
    std::string ToString() { return type.ToString(data); }
  };

}

#endif
