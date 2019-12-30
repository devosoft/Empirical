/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2019.
 *
 *  @file  DataMap2.h
 *  @brief A DataMap links names to arbitrary object types.
 *  @note Status: ALPHA
 *
 *  A DataMap links data names to arbitrary object types.  Each data map is composed of a memory
 *  image that holds a set of values and a Layout that maps names and other information to those
 *  values.
 * 
 *  Use the Add() method to include a new data entry into the DataMap.
 * 
 *  Use the Get() method to retrieve reference to a value in the DataMap.
 * 
 *  Use the Set() method to change a value in the DataMap
 *  (you may also use Get() followed by an assignment.)
 * 
 *  New data entries can be added to a DataMap, but never removed (for efficiency purposes).
 *  When a DataMap is copied, all data entries are also copied (relatively fast).
 *  As long as a DataMaps layout doesn't change, all copied maps will share the same layout (fast). 
 * 
 * 
 *  DEVELOPER NOTES:
 *  - Each DataMap can have a Layout that's internally managed.  Copied maps share a Layout.
 *    Layouts are automatically deleted when all maps that use them are gone.
 *  - When a Layout is added to, first check if it has already advanced (in a different map)
 *    beyond the current mappings.  If so, duplicate the Layout.
 *  - Layouts should be freezable to ensure that no new maps change the Layout.
 *  - Simple helper functions:
 *     Set(Layout, value)
 *     Get/SetValue, String, etc
 *  - AddLog() instead of Add() if you want to keep a set of values.  This should take flags to
 *    indicate how values should be retrieved by default, such as First, Last, Average, etc.
 *  - Settings for all entries should have more information on how they are dealt with, such as if
 *    they should be included in output an how.
 * 
 *  - After everything else is working, build a LocalDataMap<size_t> that locks in the size at
 *    compiletime, providing more localized memory.  Otherwise DataMap as a whole can be built
 *    on a templated class that takes an IMAGE_T as an argument.
 */

#ifndef EMP_DATA_MAP_H
#define EMP_DATA_MAP_H

#include <string>
#include <unordered_map>
#include <cstring>        // For std::memcpy

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../meta/TypeID.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  class DataMap {
  protected:

    class Layout {
    public:
      struct SettingInfo {
        emp::TypeID type;   ///< Type name as converted to string by typeid()
        std::string name;   ///< Name of this setting.
        std::string desc;   ///< Full description of this setting.
        std::string notes;  ///< Any additional notes about this setting.

        bool is_log;        ///< Is this setting a current value or a log of all values?
      };

    protected:
      std::unordered_map<std::string, size_t> id_map;       ///< Lookup vector positions by name.
      std::unordered_map<size_t, SettingInfo> setting_map;  ///< Lookup setting info by id.
      size_t image_size;                                    ///< What size image is expected? @CAO COMPLETE!

      /// Collect all of the constructors and destructors that we need to worry about.
      using copy_fun_t = std::function<void(const DataMap &, DataMap &)>;
      using move_fun_t = std::function<void(DataMap &, DataMap &)>;
      using destruct_fun_t = std::function<void(DataMap &)>;
      emp::vector<copy_fun_t> copy_constructors;
      emp::vector<move_fun_t> move_constructors;
      emp::vector<destruct_fun_t> destructors;

    public:
      Layout() = default;
      Layout(const Layout &) = default;
      Layout(Layout &&) = default;
      ~Layout() { ; }

      // Layouts should never change out from under a DataMap.
      Layout & operator=(const Layout &) = delete;
      Layout & operator=(Layout &&) = delete;

      /// Return the number of bytes in the default image.
      size_t GetImageSize() const { return image_size; }

      /// Lookup the unique idea for an entry.
      size_t GetID(const std::string & name) const {
        emp_assert(Has(id_map, name), name);
        return id_map.find(name)->second;
      }

      /// Lookup the type of an entry by ID.
      emp::TypeID GetType(size_t id) const {
        emp_assert(Has(setting_map, id), id);
        return setting_map.find(id)->second.type;
      }

      /// Lookup the type of an entry by name.
      emp::TypeID GetType(const std::string & name) const {
        emp_assert(Has(id_map, name), name);
        return GetType(GetID(name));
      }

      /// Add a new variable with a specified type, name and value.
      template <typename T>
      size_t Add(DataMap & base_map,
                 const std::string & name,
                 const T & default_value,
                 const std::string & desc="",
                 const std::string & notes="") {
        emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.

        // Analyze the size of the new object and where it will go.
        constexpr const size_t obj_size = sizeof(T);
        const size_t pos = image_size;

        // Create a new image with enough room for the new object.
        DataMap new_image(*this, image_size + obj_size);

        // Move the memory from the old image to the new one with more space.
        MoveImageContents(base_map, new_image);

        // Cleanup the old image.
        ClearImage(base_map);

        // Put the new image in place (and let its old version destruct cleanly)
        base_map.memory = new_image.memory;
        base_map.mem_size = new_image.mem_size;
        new_image.memory = nullptr;
        new_image.mem_size = 0;
        image_size = base_map.mem_size;

        // Setup the default version of this object.
        base_map.template Construct<T>(pos, default_value);

        // Store the information about this object.
        id_map[name] = pos;
        setting_map[pos] = { emp::GetTypeID<T>(), name, desc, notes, false };

        // Store copy constructor if needed.
        if (std::is_trivially_copyable<T>() == false) {
          copy_constructors.push_back(
            [pos](const DataMap & from_image, DataMap & to_image) {
              to_image.template CopyObj<T>(pos, from_image);
            }
          );
        }

        // Store destructor if needed.
        if (std::is_trivially_destructible<T>() == false) {
          destructors.push_back(
            [pos](DataMap & image) { image.template Destruct<T>(pos); }
          );
        }

        // Store move constructor if needed.
        if (std::is_trivially_destructible<T>() == false) {
          move_constructors.push_back(
            [pos](DataMap & from_image, DataMap & to_image) {
              to_image.template MoveObj<T>(pos, from_image);
            }
          );
        }

        return pos;
      }

      /// Detemine if we have the correct type of a specific variable ID.
      template <typename T>
      bool IsType(size_t id) const {
        emp_assert(Has(setting_map, id), id);
        return setting_map.find(id)->second.type == emp::GetTypeID<T>();
      }

      /// Retrieve a variable from a provided image by its type and position.
      template <typename T>
      T & Get(DataMap & image, size_t id) {
        emp_assert(emp::Has(setting_map, id), setting_map.size(), id, image_size);
        emp_assert(IsType<T>(id));
        image.template Get<T>(id);
      }

      /// Retrieve a const variable from an image by its type and position.
      template <typename T>
      const T & Get(DataMap & image, size_t id) const {
        emp_assert(emp::Has(setting_map, id), setting_map.size(), id, image_size);
        emp_assert(IsType<T>(id));
        image.template Get<T>(id);
      }


      // == Retrivals by name instead of by ID (slower!) ==


      /// Retrieve a variable from an image by its type and position.
      template <typename T>
      T & Get(DataMap & image, std::string & name) {
        emp_assert(emp::Has(id_map, name));
        image.template Get<T>(GetID(name));
      }

      /// Retrieve a const variable from an image by its type and position.
      template <typename T>
      const T & Get(DataMap & image, std::string & name) const {
        emp_assert(emp::Has(id_map, name));
        image.template Get<T>(GetID(name));
      }



      // -- Manipulations of images --

      /// Run destructors on all objects in a memory image (but otherwise leave it intact.)
      void DestructImage(DataMap & image) const {
        // If there is no memory in the image, stop.
        if (!image.memory) return;

        // Run destructor on contents of image and then empty it!
        for (auto & d : destructors) { d(image); }
      }

      /// Destruct and delete all memomry assocated with this DataMap.
      void ClearImage(DataMap & image) const {
        // If this DataMap is already clear, stop.
        if (!image.memory) return;

        // Run destructor on contents of image and then empty it!
        for (auto & d : destructors) { d(image); }

        // Clean up image object.
        image.memory.DeleteArray();
        image.memory = nullptr;
        image.mem_size = 0;
      }

      void CopyImage(const DataMap & from_image, DataMap & to_image) const {
        emp_assert(from_image.layout_ptr == to_image.layout_ptr);

        DestructImage(to_image);

        // Transfer over the from image and then run the required copy constructors.
        to_image.RawResize(from_image.mem_size);
        to_image.RawCopy(from_image);
        for (auto & c : copy_constructors) { c(from_image, to_image); }
      }

      // Move contents from one image to another.  Size must already be setup!
      void MoveImageContents(DataMap & from_image, DataMap & to_image) const {
        emp_assert(to_image.GetSize() >= from_image.GetSize());
        emp_assert(from_image.layout_ptr == to_image.layout_ptr);

        DestructImage(to_image);

        // Transfer over the from image and then run the required copy constructors.
        to_image.RawCopy(from_image);
        for (auto & c : move_constructors) { c(from_image, to_image); }
      }

    };

    emp::Ptr<std::byte> memory = nullptr;
    size_t mem_size = 0;
    emp::Ptr<Layout> layout_ptr;

    DataMap(emp::Ptr<Layout> in_layout_ptr, size_t in_size) : layout_ptr(in_layout_ptr) {
      if (in_size) RawResize(in_size);
    }

  public:  // Available to users of a DataMap.
    DataMap(Layout & in_layout_ptr) : layout_ptr(&in_layout_ptr) {
      layout_ptr->Initialize(*this);
    }
    DataMap(const DataMap & in_image)
    : mem_size(in_image.mem_size), layout_ptr(in_image.layout_ptr) {
      layout_ptr->CopyImage(in_image, *this);
    }
    DataMap(DataMap && in_image)
    : memory(in_image.memory), mem_size(in_image.mem_size), layout_ptr(in_image.layout_ptr) {
      in_image.memory = nullptr;
      in_image.mem_size = 0;
    }

    ~DataMap() {
      if (memory) memory.DeleteArray();
    }

    /// Retrieve the Layout associated with this image.
    Layout & GetMapLayout() { return *layout_ptr; }
    const Layout & GetMapLayout() const { return *layout_ptr; }

    /// Determine how many Bytes large this image is.
    size_t GetSize() const { return mem_size; }

    /// Is this image using the most current version of the Layout?
    bool IsCurrent() const { return mem_size == layout_ptr->GetImageSize(); }

    /// Get a typed pointer to a specific position in this image.
    template <typename T> emp::Ptr<T> GetPtr(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      return reinterpret_cast<T*>(&memory[id]);
    }

    /// Get a proper reference to an object represented in this image.
    template <typename T> T & Get(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      return *(reinterpret_cast<T*>(&memory[id]));
    }

    /// Get a const reference to an object represented in this image.
    template <typename T> const T & Get(size_t id) const {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      return *(reinterpret_cast<const T*>(&memory[id]));
    }

    std::byte & operator[](size_t id) {
      emp_assert(id < GetSize(), id, GetSize());
      return memory[id];
    }
    const std::byte & operator[](size_t id) const {
      emp_assert(id < GetSize(), id, GetSize());
      return memory[id];
    }

  protected: // Only available to friend class Layout

    /// Change the size of this memory.  Assume all cleanup and setup is done elsewhere.
    void RawResize(size_t new_size) {
      // If the size is already good, stop here.
      if (mem_size == new_size) return;

      if (memory) memory.DeleteArray();  // If there was memory here, free it.
      mem_size = new_size;               // Determine the new size.
      memory.NewArray(mem_size);         // Allocate the new space.
    }


    /// Copy all of the bytes directly from another memory image.  Size manipulation must be
    /// done beforehand to ensure sufficient space is availabe.
    void RawCopy(const DataMap & in_image) {
      emp_assert(mem_size >= in_image.mem_size);
      if (in_image.mem_size == 0) return; // Nothing to copy!

      // Copy byte-by-byte into this memory.
      std::memcpy(memory.Raw(), in_image.memory.Raw(), mem_size);
    }

    /// Build a new object of the provided type at the memory position indicated.
    template <typename T, typename... ARGS>
    void Construct(size_t id, ARGS &&... args) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      new (GetPtr<T>(id).Raw()) T( std::forward<ARGS>(args)... );
    }

    /// Destruct an object of the provided type at the memory position indicated; don't release memory!
    template <typename T>
    void Destruct(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      GetPtr<T>(id)->~T();
    }

    /// Copy an object from another DataMap with an identical layout.
    template<typename T>
    void CopyObj(size_t id, const DataMap & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(from_image.layout_ptr == layout_ptr);
      Construct<T, const T &>(id, from_image.Get<T>(id));
    }

    /// Move an object from another DataMap with an identical layout.
    template<typename T>
    void MoveObj(size_t id, DataMap & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      emp_assert(from_image.layout_ptr == layout_ptr);
      Construct<T, const T &>(id, std::move(from_image.Get<T>(id)));  // Move the object.
      from_image.Destruct<T>(id);                                         // Destruct old version.
    }

  };

}

#endif
