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
 *  image that holds a set of values and a DataLayout that maps names and other information to
 *  those values.
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

  /// A helper class for DataMap; tracks a memory image (sequence of bytes).

  class MemoryImage {
  private:
    emp::Ptr<std::byte> image = nullptr;
    size_t size = 0;

  public:
    MemoryImage() = default;
    MemoryImage(size_t in_size) : image( emp::NewArrayPtr<std::byte>(in_size) ), size(in_size) { ; }
    ~MemoryImage() { if (image) image.DeleteArray(); }

    size_t GetSize() const { return size; }

    /// Get a typed pointer to a specific position in this image.
    template <typename T> emp::Ptr<T> GetPtr(size_t pos) {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return reinterpret_cast<T*>(&image[pos]);
    }

    template <typename T> emp::Ptr<const T> GetPtr(size_t pos) const {
      emp_assert(pos + sizeof(T) <= GetSize(), pos, sizeof(T), GetSize());
      return reinterpret_cast<T const *>(&image[pos]);
    }

    /// Get proper references to an object represented in this image.
    template <typename T> T & Get(size_t pos) { return *GetPtr<T>(pos); }
    template <typename T> const T & Get(size_t pos) const { return *GetPtr<T>(pos); }

    /// Change the size of this memory.  Assume all cleanup and setup is done elsewhere.
    void RawResize(size_t new_size) {
      // If the size is already good, stop here.
      if (GetSize() == new_size) return;

      if (image) image.DeleteArray();   // If there was memory here, free it.
      size = new_size;                  // Determine the new size.
      if (size) image.NewArray(size);   // Allocate the new space.
      else image = nullptr;             // Or set to null if now empty.
    }

    /// Copy all of the bytes directly from another memory image.  Size manipulation must be
    /// done beforehand to ensure sufficient space is availabe.
    void RawCopy(const MemoryImage & in_memory) {
      emp_assert(GetSize() >= in_memory.GetSize());
      if (in_memory.GetSize() == 0) return; // Nothing to copy!

      // Copy byte-by-byte into this memory.
      std::memcpy(image.Raw(), in_memory.image.Raw(), in_memory.GetSize());
    }

    /// Steal the memory from the image passed in.  Current memory should have been cleaned up
    /// and set to null already.
    void RawMove(MemoryImage & in_memory) {
      emp_assert(image.IsNull());
      image = in_memory.image;
      size = in_memory.size;
      in_memory.image = nullptr;
      in_memory.size = 0;
    }

    /// Build a new object of the provided type at the memory position indicated.
    template <typename T, typename... ARGS>
    void Construct(size_t id, ARGS &&... args) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      new (GetPtr<T>(id).Raw()) T( std::forward<ARGS>(args)... );
    }

    /// Destruct an object of provided type at memory position indicated; don't release memory!
    template <typename T>
    void Destruct(size_t id) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      GetPtr<T>(id)->~T();
    }

    /// Copy an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void CopyObj(size_t id, const MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      Construct<T, const T &>(id, from_image.Get<T>(id));
    }

    /// Move an object from another MemoryImage with an identical DataLayout.
    template<typename T>
    void MoveObj(size_t id, MemoryImage & from_image) {
      emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
      Construct<T, const T &>(id, std::move(from_image.Get<T>(id)));     // Move the object.
      from_image.Destruct<T>(id);                                        // Destruct old version.
    }
  };


  /// A helper class for DataMap; tracks how data is organized in a MemoryImage.

  class DataLayout {
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
    size_t image_size;                                    ///< What size image is expected?

    size_t num_maps = 1;                                  ///< How many DataMaps use this layout?

    /// Collect all of the constructors and destructors that we need to worry about.
    using copy_fun_t = std::function<void(const MemoryImage &, MemoryImage &)>;
    using move_fun_t = std::function<void(MemoryImage &, MemoryImage &)>;
    using destruct_fun_t = std::function<void(MemoryImage &)>;
    emp::vector<copy_fun_t> copy_constructors;
    emp::vector<move_fun_t> move_constructors;
    emp::vector<destruct_fun_t> destructors;

  public:
    DataLayout() = default;
    DataLayout(const DataLayout &) = default;
    DataLayout(DataLayout &&) = default;
    ~DataLayout() { ; }

    // Existing layouts should never change out from under a DataMap.
    DataLayout & operator=(const DataLayout &) = delete;
    DataLayout & operator=(DataLayout &&) = delete;

    void IncMaps() { num_maps++; }
    void DecMaps() { num_maps--; }
    size_t GetNumMaps() const { return num_maps; }

    /// Determine if we have a variable by a given name.
    bool HasName(const std::string & name) const { return emp::Has(id_map, name); }

    /// Determine if we have an ID.
    bool HasID(size_t id) const { return emp::Has(setting_map, id); }

    /// Detemine if we have the correct type of a specific variable ID.
    template <typename T>
    bool IsType(size_t id) const {
      emp_assert(Has(setting_map, id), id);
      return setting_map.find(id)->second.type == emp::GetTypeID<T>();
    }


    /// Return the number of bytes in the default image.
    size_t GetImageSize() const { return image_size; }

    /// Lookup the unique idea for an entry.
    size_t GetID(const std::string & name) const {
      emp_assert(HasName(name), name);
      return id_map.find(name)->second;
    }

    /// Lookup the type of an entry by ID.
    emp::TypeID GetType(size_t id) const {
      emp_assert(HasID(id), id);
      return setting_map.find(id)->second.type;
    }


    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t Add(MemoryImage & base_memory,
                const std::string & name,
                const T & default_value,
                const std::string & desc="",
                const std::string & notes="") {
      emp_assert(!HasName(name), name);               // Make sure this doesn't already exist.

      // Analyze the size of the new object and where it will go.
      constexpr const size_t obj_size = sizeof(T);
      const size_t pos = image_size;

      // Create a new image with enough room for the new object and move the old data over.
      MemoryImage new_memory(image_size + obj_size);
      MoveImageContents(base_memory, new_memory);

      // Now that the data is moved, cleanup the old image and put the new one in place.
      ClearImage(base_memory);
      base_memory.RawMove(new_memory);
      image_size = base_memory.GetSize();

      // Setup the default version of this new object.
      base_memory.Construct<T>(pos, default_value);

      // Store the information about this object.
      id_map[name] = pos;
      setting_map[pos] = { emp::GetTypeID<T>(), name, desc, notes, false };

      // Store copy constructor if needed.
      if (std::is_trivially_copyable<T>() == false) {
        copy_constructors.push_back(
          [pos](const MemoryImage & from_image, MemoryImage & to_image) {
            to_image.CopyObj<T>(pos, from_image);
          }
        );
      }

      // Store destructor if needed.
      if (std::is_trivially_destructible<T>() == false) {
        destructors.push_back(
          [pos](MemoryImage & image) { image.Destruct<T>(pos); }
        );
      }

      // Store move constructor if needed.
      if (std::is_trivially_destructible<T>() == false) {
        move_constructors.push_back(
          [pos](MemoryImage & from_image, MemoryImage & to_image) {
            to_image.MoveObj<T>(pos, from_image);
          }
        );
      }

      return pos;
    }

    // -- Manipulations of images --

    /// Run destructors on all objects in a memory image (but otherwise leave it intact.)
    void DestructImage(MemoryImage & image) const {
      // If there is no memory in the image, stop.
      if (image.GetSize() == 0) return;

      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(image); }
    }

    /// Destruct and delete all memomry assocated with this DataMap.
    void ClearImage(MemoryImage & memory) const {
      // If this memory image is already clear, stop.
      if (memory.GetSize() == 0) return;

      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(memory); }

      // Clean up image memory.
      memory.RawResize(0);
    }

    void CopyImage(const MemoryImage & from_image, MemoryImage & to_image) const {
      DestructImage(to_image);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawResize(from_image.GetSize());
      to_image.RawCopy(from_image);
      for (auto & c : copy_constructors) { c(from_image, to_image); }
    }

    // Move contents from one image to another.  Size must already be setup!
    void MoveImageContents(MemoryImage & from_image, MemoryImage & to_image) const {
      emp_assert(to_image.GetSize() >= from_image.GetSize());

      DestructImage(to_image);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawCopy(from_image);
      for (auto & c : move_constructors) { c(from_image, to_image); }
    }

  };


  class DataMap {
  protected:
    MemoryImage memory;
    emp::Ptr<DataLayout> layout_ptr;

    DataMap(emp::Ptr<DataLayout> in_layout_ptr, size_t in_size)
      : memory(in_size), layout_ptr(in_layout_ptr) { ; }

    void CopyImage(const DataMap & from_map, DataMap & to_map) {
      emp_assert(from_map.layout_ptr == to_map.layout_ptr);
      layout_ptr->CopyImage(from_map.memory, to_map.memory);
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
      emp_assert(HasID(id), id, GetSize());
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
      return memory.Get<T>(GetID(name));
    }

    /// Retrieve a const variable by its type and name. (Slower!)
    template <typename T>
    const T & Get(const std::string & name) const {
      emp_assert(HasName(name));
      return memory.Get<T>(GetID(name));
    }

    /// Look up the type of a variable by ID.
    emp::TypeID GetType(size_t id) const { return layout_ptr->GetType(id); }

    /// Look up the type of a variable by name.
    emp::TypeID GetType(const std::string & name) const { return layout_ptr->GetType(GetID(name)); }


    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t Add(const std::string & name,
               const T & default_value,
               const std::string & desc="",
               const std::string & notes="") {
      return layout_ptr->Add<T>(memory, name, default_value, desc, notes);
    }
  };

}

#endif
