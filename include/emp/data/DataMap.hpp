/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2020.
 *
 *  @file  DataMap.hpp
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
 *  - Each entry can have a one-byte control block immediately proceeding it in memory.  Each
 *    bit would be associated with additional information about the entry.  Options include:
 *    1. The memory is a POINTER not an instance.  This would allow entries to behave like
 *       references, potentially eliminating the need to copy larger data structures into the
 *       memory image.
 *    2. The entry has a non-trivial (or user-provided) COPY/MOVE CONSTRUCTOR or DESTRUCTOR
 *    3. The entry has a function to call for a Get instead of a value in memory.  The space
 *       reserved is used for the function pointer (incompatible with bit 1...)
 *    4. The entry has a function to call when it is set.  Effectively this can implement SIGNAL
 *       monitoring it that should be notified whenever it changes.  The signal itself would need
 *       to be stored elsewhere (presumably in the memory image, but possibly in the layout.)
 *    5. The memory is a LOG of values, not a single value.  This allows for quick identification
 *       of when something special needs to be done.
 *    6-8. Limited type information (8 types that can be handled more effectively?)
 *
 *  - We should be able to keep a series of values, not just a single one.  This can be done with
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
 *
 *  - An OptimizeLayout() function that can reorder entries so that they are somehow more sensible?
 *
 *  - A MemoryImage factory to speed up allocation, deallocation if we're using the same size
 *    images repeatedly.
 *
 *  - Some way of grouping memory across DataMaps so that a particular entry for many maps has all
 *    of its instances consecutive in memory?  This seems really tricky to pull of, but if we can
 *    do it, the improvement in cache performance could be dramatic.
 */

#ifndef EMP_DATA_MAP_H
#define EMP_DATA_MAP_H

#include <string>
#include <cstring>        // For std::memcpy

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../meta/TypeID.hpp"
#include "../tools/string_utils.hpp"

#include "MemoryImage.hpp"
#include "DataLayout.hpp"

namespace emp {

  class DataMap {
  protected:
    MemoryImage memory;              ///< Memory status for this Map.
    emp::Ptr<DataLayout> layout_ptr; ///< Which layout are we using?

    DataMap(emp::Ptr<DataLayout> in_layout_ptr, size_t in_size)
      : memory(in_size), layout_ptr(in_layout_ptr) { ; }

    /// If the current layout is shared, make a copy of it.
    void MakeLayoutUnique() {
      // Make sure we have a layout, even if empty.
      if (layout_ptr.IsNull()) layout_ptr = emp::NewPtr<DataLayout>();

      // If our we already had layout and it is shared, make a copy.
      else if (layout_ptr->GetNumMaps() > 1) {
        layout_ptr->DecMaps();
        layout_ptr.New(*layout_ptr);
      }
    }
  public:
    DataMap() : layout_ptr(nullptr) { ; }
    DataMap(const DataMap & in_map) : layout_ptr(in_map.layout_ptr) {
      if (layout_ptr) {
        layout_ptr->CopyImage(in_map.memory, memory);
        layout_ptr->IncMaps();
      }
    }
    DataMap(DataMap && in_map) : memory(std::move(in_map.memory)), layout_ptr(in_map.layout_ptr) {
      in_map.memory.RawResize(0);
      // @CAO: Should we set in_map.layout_ptr to null???
    }

    // Copy Operator...
    DataMap & operator=(const DataMap & in_map) {
      // If we have a layout pointer, use it to clear our memory image and update it if needed.
      if (layout_ptr) {
        layout_ptr->ClearImage(memory);

        // If layout pointer doesn't match the new one, shift over.
        if (layout_ptr != in_map.layout_ptr) {
          layout_ptr->DecMaps();                                   // Remove self from counter.
          if (layout_ptr->GetNumMaps() == 0) layout_ptr.Delete();  // Delete layout if now unused.
          layout_ptr = in_map.layout_ptr;                          // Shift to new layout.
          if (layout_ptr) layout_ptr->IncMaps();                   // Add self to new counter.
        }
      }

      // Otherwise we DON'T have a layout pointer, so setup the new one.
      else {
        layout_ptr = in_map.layout_ptr;                            // Shift to new layout.
        if (layout_ptr) layout_ptr->IncMaps();                     // Add self to new counter.
      }

      // Now that we know we have a good layout, copy over the image.
      layout_ptr->CopyImage(in_map.memory, memory);

      return *this;
    }

    ~DataMap() {
      /// If we have a layout pointer, clean up!
      if (!layout_ptr.IsNull()) {
        // Clean up the current MemoryImage.
        layout_ptr->ClearImage(memory);

        // Clean up the DataLayout
        layout_ptr->DecMaps();
        if (layout_ptr->GetNumMaps() == 0) layout_ptr.Delete();
      }
    }

    /// Determine how many Bytes large this image is.
    size_t GetSize() const { return memory.GetSize(); }

    /// Translate a name into an ID.
    size_t GetID(const std::string & name) const {
      emp_assert(layout_ptr);
      return layout_ptr->GetID(name);
    }

    /// Test if this map has a setting ID.
    bool HasID(size_t id) const {
      return layout_ptr && layout_ptr->HasID(id);
    }

    /// Test is this map has a variable by a given name.
    bool HasName(const std::string & name) const { return layout_ptr && layout_ptr->HasName(name); }

    /// Test if a variable is of a given type.
    template <typename T> bool IsType(size_t id) const {
      emp_assert(layout_ptr);
      return layout_ptr->IsType<T>(id);
    }
    template <typename T> bool IsType(const std::string & name) const {
      emp_assert(layout_ptr);
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
      emp_assert(HasName(name), name);
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

    /// Look up the type of a variable by ID.
    emp::TypeID GetType(size_t id) const {
      emp_assert(layout_ptr);
      return layout_ptr->GetType(id);
    }

    /// Look up the type of a variable by name.
    emp::TypeID GetType(const std::string & name) const {
      emp_assert(layout_ptr);
      return layout_ptr->GetType(GetID(name));
    }

    /// Get the memory at the target position, assume it is the provided type, and convert the
    /// value found there to double.
    double GetAsDouble(size_t id, TypeID type_id) const {
      emp_assert(HasID(id), "Can only Get IDs that are available in DataMap.", id, GetSize());
      emp_assert(type_id == layout_ptr->GetType(id));
      return type_id.ToDouble(memory.GetPtr(id));
    }

    /// Get the memory at the target position, lookup it's type, and convert the value to double.
    double GetAsDouble(size_t id) const {
      emp_assert(HasID(id), "Can only get IDs the are available in DataMap.", id, GetSize());
      return GetAsDouble(id, layout_ptr->GetType(id));
    }

    /// Get the memory at the target position, assume it is the provided type, and convert the
    /// value found there to string.
    std::string GetAsString(size_t id, TypeID type_id) const {
      emp_assert(HasID(id), "Can only Get IDs that are available in DataMap.", id, GetSize());
      emp_assert(type_id == layout_ptr->GetType(id));
      return type_id.ToString(memory.GetPtr(id));
    }

    /// Get the memory at the target position, lookup it's type, and convert the value to string.
    std::string GetAsString(size_t id) const {
      emp_assert(HasID(id), "Can only get IDs the are available in DataMap.", id, GetSize());
      return GetAsString(id, layout_ptr->GetType(id));
    }

    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t AddVar(const std::string & name,
               const T & default_value,
               const std::string & desc="",
               const std::string & notes="") {
      MakeLayoutUnique();  // If the current layout is shared, first make a copy of it.
      return layout_ptr->Add<T>(memory, name, default_value, desc, notes);
    }

    /// Test if this DataMap is using the identical layout as another DataMap.
    bool SameLayout(const emp::DataMap & in_dm) const {
      return layout_ptr == in_dm.layout_ptr;
      // @CAO: Should we also see if it's using a different layout object, but otherwise identical?
    }

    /// Test if this layout is locked (i.e., it cannot be changed.)
    bool IsLocked() const { return layout_ptr && layout_ptr->IsLocked(); }

    /// Prevent this DataMap's layout from having any additional changed made to it.
    void LockLayout() {
      MakeLayoutUnique();
      layout_ptr->Lock();
    }
  };

}

#endif
