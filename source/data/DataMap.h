/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2020.
 *
 *  @file  DataMap.h
 *  @brief A DataMap links names to arbitrary object types.
 *  @note Status: ALPHA
 *
 *  A DataMap links data names to arbitrary object types.  Each data map is composed of a
 *  MemoryImage (that holds a set of values) and a DataLayout (that maps names and other info
 *  to those values.)
 * 
 *  AddVar<type>("name", value, ["desc"], ["notes"])
 *   Includes a new data entry into the DataMap and returns its uniquq ID.
 * 
 *  Get<type>("name")   - retrieve a reference to a value in the DataMap slowly.
 *  Get<type>(ID)       - retrieve a reference more quickly.
 *  GetID("name")       - convert a name into a unique ID.
 *  Set(name|id, value) - change a value in the DataMap
 *    (you may also use Get() followed by an assignment.)
 * 
 *  New data entries can be added to a DataMap, but never removed (for efficiency purposes).
 *  When a DataMap is copied, all data entries are also copied (relatively fast).
 *  As long as a DataMaps layout doesn't change, all copied maps will share the same layout (fast). 
 * 
 *  A layout can also be locked with LockLayout(), which will throw an error if there is an attempt
 *  to modify that layout again.  A lock can be checked with IsLocked().
 * 
 *  Specialty versions of Get and Set exist if you don't want to use templates for simple types.
 *  They are GetValue(*), SetValue(*), GetString(*), and SetString(*).  Values are all represented
 *  as doubles.
 * 
 * 
 *  DEVELOPER NOTES:
 *  - We should be able to keep a series of values, not just a single on.  This can be done with
 *    a series of new functions:
 *      AddLog() instead of AddVar() when new veriable is created.
 *      Get() should still work for latest value.  Ideally keep lates in first position.
 *      Change non-const Get() to GetRef() which cannot be used for a log.
 *      Add GetAve() function for logs as well as GetLog() for the full vector.
 * 
 *  - Settings for all entries should have more information on how they are dealt with, such as if
 *    they should be included in output an how.  Perhaps a system of tags for dynamic use?
 * 
 *  - After everything else is working, build a LocalDataMap<size_t> that locks in the size at
 *    compile time, providing more localized memory.  Otherwise DataMap as a whole can be built
 *    on a templated class that takes an IMAGE_T as an argument.
 * 
 *  - Default values should be saved in the layout allowing any MemoryImage to be easily reset to
 *    factory settings.
 * 
 *  - A user should be able to override copy constructors (though probably not move constructors
 *    or destructors?).  Then the copy process can be more customizable, for example having some
 *    settings retrun to the default value or be further processed.  It's also possible to have
 *    multiple types of copies, so if we indicate a "Copy birth" we get the above, but if we
 *    indicate a "Copy clone" or "Copy inject" we do something different.  We also probably need
 *    to allow for multiple parents...
 */

#ifndef EMP_DATA_MAP_H
#define EMP_DATA_MAP_H

#include <string>
#include <cstring>        // For std::memcpy

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../meta/TypeID.h"
#include "../tools/string_utils.h"

#include "MemoryImage.h"
#include "DataLayout.h"

namespace emp {

  class DataMap {
  protected:
    MemoryImage memory;              ///< Memory status for this Map.
    emp::Ptr<DataLayout> layout_ptr; ///< Which layout are we using?

    DataMap(emp::Ptr<DataLayout> in_layout_ptr, size_t in_size)
      : memory(in_size), layout_ptr(in_layout_ptr) { ; }

    void CopyImage(const DataMap & from_map, DataMap & to_map) {
      emp_assert(from_map.layout_ptr == to_map.layout_ptr);
      layout_ptr->CopyImage(from_map.memory, to_map.memory);
    }

    /// If the current layout is shared, make a copy of it.
    void MakeLayoutUnique() {
      if (layout_ptr->GetNumMaps() > 1) {
        layout_ptr->DecMaps();
        layout_ptr.New(*layout_ptr);
      }
    }
  public:
    DataMap() : layout_ptr(emp::NewPtr<DataLayout>()) { ; }
    DataMap(const DataMap & in_map) : layout_ptr(in_map.layout_ptr) {
      CopyImage(in_map, *this);
      layout_ptr->IncMaps();
    }
    DataMap(DataMap && in_map) : memory(std::move(in_map.memory)), layout_ptr(in_map.layout_ptr) {
      in_map.memory.RawResize(0);
    }

    ~DataMap() {
      // Clean up the current MemoryImage.
      layout_ptr->ClearImage(memory);

      // Clean up the DataLayout
      layout_ptr->DecMaps();
      if (layout_ptr->GetNumMaps() == 0) layout_ptr.Delete();
    }

    /// Retrieve the DataLayout associated with this image.
    DataLayout & GetMapLayout() { return *layout_ptr; }
    const DataLayout & GetMapLayout() const { return *layout_ptr; }

    /// Determine how many Bytes large this image is.
    size_t GetSize() const { return memory.GetSize(); }

    /// Translate a name into an ID.
    size_t GetID(const std::string & name) const { return layout_ptr->GetID(name); }

    /// Is this image using the most current version of the DataLayout?
    bool IsCurrent() const { return GetSize() == layout_ptr->GetImageSize(); }

    /// Test if this map has a setting ID.
    bool HasID(size_t id) const { return layout_ptr->HasID(id); }

    /// Test is this map has a variable by a given name.
    bool HasName(const std::string & name) const { return layout_ptr->HasName(name); }

    /// Test if a variable is of a given type.
    template <typename T> bool IsType(size_t id) const { return layout_ptr->IsType<T>(id); }
    template <typename T> bool IsType(const std::string & name) const {
      return layout_ptr->IsType<T>(GetID(name));
    }

    /// Retrieve a variable by its type and position.
    template <typename T>
    T & Get(size_t id) {
      emp_assert(HasID(id), "Can only get IDs the are available in DataMap.", id, GetSize());
      emp_assert(IsType<T>(id));
      return memory.Get<T>(id);
    }

    /// Retrieve a const variable by its type and position.
    template <typename T>
    const T & Get(size_t id) const {
      emp_assert(HasID(id), id, GetSize());
      emp_assert(IsType<T>(id));
      return memory.Get<T>(id);
    }


    /// Retrieve a variable by its type and name. (Slower!)
    template <typename T>
    T & Get(const std::string & name) {
      emp_assert(HasName(name));
      emp_assert(IsType<T>(name), "DataMap::Get() must be provided the correct type.",
                 name, GetType(name), emp::GetTypeID<T>());
      return memory.Get<T>(GetID(name));
    }

    /// Retrieve a const variable by its type and name. (Slower!)
    template <typename T>
    const T & Get(const std::string & name) const {
      emp_assert(HasName(name));
      emp_assert(IsType<T>(name), name, GetType(name), emp::GetTypeID<T>());
      return memory.Get<T>(GetID(name));
    }

    /// Set a variable by ID.
    template <typename T> T & Set(size_t id, const T & value) {
      return (Get<T>(id) = value);
    }

    /// Set a variable by name.
    template <typename T> T & Set(const std::string & name, const T & value) {
      return (Get<T>(name) = value);
    }

    // Type-specific Getters and Setters
    double & GetValue(size_t id) { return Get<double>(id); }
    double GetValue(size_t id) const { return Get<double>(id); }
    double & GetValue(const std::string & name) { return Get<double>(name); }
    double GetValue(const std::string & name) const { return Get<double>(name); }
    double & SetValue(size_t id, double value) { return Set<double>(id, value); }
    double & SetValue(const std::string & name, double value) { return Set<double>(name, value); }

    std::string & GetString(const size_t id) { return Get<std::string>(id); }
    const std::string & GetString(const size_t id) const { return Get<std::string>(id); }
    std::string & GetString(const std::string & name) { return Get<std::string>(name); }
    const std::string & GetString(const std::string & name) const { return Get<std::string>(name); }
    std::string & SetString(const size_t id, const std::string & value) { return Set<std::string>(id, value); }
    std::string & SetString(const std::string & name, const std::string & value) { return Set<std::string>(name, value); }

    /// Look up the type of a variable by ID.
    emp::TypeID GetType(size_t id) const { return layout_ptr->GetType(id); }

    /// Look up the type of a variable by name.
    emp::TypeID GetType(const std::string & name) const { return layout_ptr->GetType(GetID(name)); }


    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t AddVar(const std::string & name,
               const T & default_value,
               const std::string & desc="",
               const std::string & notes="") {
      MakeLayoutUnique();  // If the current layout is shared, first make a copy of it.
      return layout_ptr->Add<T>(memory, name, default_value, desc, notes);
    }

    // Add type-specific variables.
    template <typename... Ts> size_t AddStringVar(Ts &&... args) { return AddVar<std::string>(args...); }
    template <typename... Ts> size_t AddValueVar(Ts &&... args) { return AddVar<double>(args...); }

    bool IsLocked() const { return layout_ptr->IsLocked(); }

    void LockLayout() {
      MakeLayoutUnique();
      layout_ptr->Lock();
    }
  };

}

#endif
