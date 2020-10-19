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
 *  Effectively it acts as a void pointer, but with run-time type information that will
 *  allow for dynamic manipulation of the underlying value.
 *
 */

#ifndef EMP_DATA_ENTRY_H
#define EMP_DATA_ENTRY_H

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../meta/TypeID.hpp"

namespace emp {

  class DataEntry {
  protected:
    emp::Ptr<void> data = nullptr;
    emp::TypeID type;

    DataEntry() { }
    DataEntry(const DataEntry &) = default;

    DataEntry & operator=(const DataEntry &) = default;

    const std::string & GetTypeName() const { return type.GetName(); }
    TypeID GetTypeID() const { return type; }

    template <typename T>
    bool IsType() const { return type == GetTypeID<T>(); }

    double ToDouble() const { return type.ToDouble(data); }
    std::string ToString() const { return type.ToString(data); }

    template <typename T>
    void SetPtr(emp::Ptr<T> ptr) { data = ptr; }

    template <typename T>
    void SetRef(T & ref) { data = &ref; }

    bool SetValue(double value) { return type.FromDouble(value, data); }
    bool SetValue(const std::string & value) { return type.FromString(value, data); }

    template <typename T>
    T & As() { return *data.ReinterpretCast<T>(); }

    template <typename T>
    const T & As() const { return *data.ReinterpretCast<const T>(); }
  };

}

#endif
