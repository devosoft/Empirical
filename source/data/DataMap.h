/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2019.
 *
 *  @file  DataMap2.h
 *  @brief Track arbitrary data by name (slow) or id (faster).
 *  @note Status: ALPHA
 *
 *  A DataMap links to a provided memory image to maintain arbitrary object types.
 *  There are two derived types:
 *    DataArray is fast, but requires size to be provided at compile time.
 *    DataVector is slower, but has a dynamic memory size.
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
  public:
    struct SettingInfo {
      emp::TypeID type;   ///< Type name as converted to string by typeid()
      std::string name;   ///< Name of this setting.
      std::string desc;   ///< Full description of this setting.
      std::string notes;  ///< Any additional notes about this setting.
    };

    class MemoryImage {
      friend DataMap;
    protected:
      emp::Ptr<std::byte> memory = nullptr;
      size_t mem_size = 0;
      DataMap & data_map;

      MemoryImage(DataMap & in_data_map, size_t in_size) : data_map(in_data_map) {
        if (in_size) RawResize(in_size);
      }

    public:  // Available to users of a MemoryImage.
      MemoryImage(DataMap & in_data_map) : data_map(in_data_map) {
        data_map.Initialize(*this);
      }
      MemoryImage(const MemoryImage & in_image)
      : mem_size(in_image.mem_size), data_map(in_image.data_map) {
        data_map.CopyImage(in_image, *this);
      }
      MemoryImage(MemoryImage && in_image)
      : memory(in_image.memory), mem_size(in_image.mem_size), data_map(in_image.data_map) {
        in_image.memory = nullptr;
        in_image.mem_size = 0;
      }

      ~MemoryImage() {
        if (memory) memory.DeleteArray();
      }

      /// Retrieve the DataMap associated with this image.
      DataMap & GetDataMap() { return data_map; }
      const DataMap & GetDataMap() const { return data_map; }

      /// Determine how many Bytes large this image is.
      size_t GetSize() const { return mem_size; }

      /// Is this image using the most current version of the DataMap?
      bool IsCurrent() const { return mem_size == data_map.GetImageSize(); }

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

    protected: // Only available to friend class DataMap

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
      void RawCopy(const MemoryImage & in_image) {
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

      /// Copy an object from another MemoryImage with an identical layout.
      template<typename T>
      void CopyObj(size_t id, const MemoryImage & from_image) {
        emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
        emp_assert(&from_image.data_map == &data_map);
        Construct<T, const T &>(id, from_image.Get<T>(id));
      }

      /// Move an object from another MemoryImage with an identical layout.
      template<typename T>
      void MoveObj(size_t id, MemoryImage & from_image) {
        emp_assert(id + sizeof(T) <= GetSize(), id, sizeof(T), GetSize());
        emp_assert(&from_image.data_map == &data_map);
        Construct<T, const T &>(id, std::move(from_image.Get<T>(id)));  // Move the object.
        from_image.Destruct<T>(id);                                         // Destruct old version.
      }

    };

  protected:
    MemoryImage default_image;                            ///< Memory image for these data.
    std::unordered_map<std::string, size_t> id_map;       ///< Lookup vector positions by name.
    std::unordered_map<size_t, SettingInfo> setting_map;  ///< Lookup setting info by id.

    /// Collect all of the constructors and destructors that we need to worry about.
    using copy_fun_t = std::function<void(const MemoryImage &, MemoryImage &)>;
    using destruct_fun_t = std::function<void(MemoryImage &)>;
    using move_fun_t = std::function<void(MemoryImage &, MemoryImage &)>;
    emp::vector<copy_fun_t> copy_constructors;
    emp::vector<move_fun_t> move_constructors;
    emp::vector<destruct_fun_t> destructors;

  public:
    DataMap() : default_image(*this, 0) { ; }
    // DataMap(const DataMap &) = default;
    // DataMap(DataMap &&) = default;
    ~DataMap() { ClearImage(default_image); }

    // DataMap & operator=(const DataMap &) = default;
    DataMap & operator=(DataMap &&) = default;

    /// Return the number of bytes in the default image.
    size_t GetImageSize() const { return default_image.GetSize(); }

    /// Get a const reference to the default image.
    const MemoryImage & GetDefaultImage() const { return default_image; }

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
    size_t Add(const std::string & name,
	             const T & default_value,
	             const std::string & desc="",
	             const std::string & notes="") {
      emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.

      // Analyze the size of the new object and where it will go.
      constexpr const size_t obj_size = sizeof(T);
      const size_t pos = default_image.GetSize();

      // Create a new image with enough room for the new object.
      MemoryImage new_image(*this, default_image.GetSize() + obj_size);

      // Move the memory from the old image to the new one with more space.
      MoveImageContents(default_image, new_image);

      // Cleanup the old image.
      ClearImage(default_image);

      // Put the new image in place (and let the old version destruct cleanly)
      default_image.memory = new_image.memory;
      default_image.mem_size = new_image.mem_size;
      new_image.memory = nullptr;
      new_image.mem_size = 0;


      // Setup the default version of this object.
      default_image.template Construct<T>(pos, default_value);

      // Store the position in the id map.
      id_map[name] = pos;

      // Store all of the other settings for this object.
      setting_map[pos] = { emp::GetTypeID<T>(), name, desc, notes };

      // Store copy constructor if needed.
      if (std::is_trivially_copyable<T>() == false) {
        copy_constructors.push_back(
          [pos](const MemoryImage & from_image, MemoryImage & to_image) {
            to_image.template CopyObj<T>(pos, from_image);
          }
        );
      }

      // Store destructor if needed.
      if (std::is_trivially_destructible<T>() == false) {
        destructors.push_back(
          [pos](MemoryImage & image) { image.template Destruct<T>(pos); }
        );
      }

      // Store move constructor if needed.
      if (std::is_trivially_destructible<T>() == false) {
        move_constructors.push_back(
          [pos](MemoryImage & from_image, MemoryImage & to_image) {
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

    /// Detemine if we have the correct type of a specific variable name.
    template <typename T>
    bool IsType(const std::string & name) const {
      return GetType(name) == emp::GetTypeID<T>();
    }

    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(size_t id) {
      emp_assert(emp::Has(setting_map, id), setting_map.size(), id, default_image.GetSize());
      emp_assert(IsType<T>(id));
      return default_image.template Get<T>(id);
    }

    template <typename T>
    T & GetDefault(const std::string & name) { return GetDefault<T>(GetID(name)); }

    /// Retrieve a variable from a provided image by its type and position.
    template <typename T>
    T & Get(MemoryImage & image, size_t id) {
      emp_assert(emp::Has(setting_map, id), setting_map.size(), id, default_image.GetSize());
      emp_assert(IsType<T>(id));
      image.template Get<T>(id);
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(size_t id) const {
      emp_assert(emp::Has(setting_map, id), setting_map.size(), id, default_image.GetSize());
      emp_assert(IsType<T>(id));
      default_image.template Get<T>(id);
    }

    template <typename T>
    const T & GetDefault(const std::string & name) const { return GetDefault<T>(GetID(name)); }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T>
    const T & Get(MemoryImage & image, size_t id) const {
      emp_assert(emp::Has(setting_map, id), setting_map.size(), id, default_image.GetSize());
      emp_assert(IsType<T>(id));
      image.template Get<T>(id);
    }


    // == Retrivals by name instead of by ID (slower!) ==


    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(std::string & name) {
      emp_assert(emp::Has(id_map, name));
      default_image.template Get<T>(GetID(name));
    }

    /// Retrieve a variable from an image by its type and position.
    template <typename T>
    T & Get(MemoryImage & image, std::string & name) {
      emp_assert(emp::Has(id_map, name));
      image.template Get<T>(GetID(name));
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      default_image.template Get<T>(GetID(name));
    }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T>
    const T & Get(MemoryImage & image, std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      image.template Get<T>(GetID(name));
    }



    // -- Manipulations of images --

    /// Run destructors on all objects in a memory image (but otherwise leave it intact.)
    void DestructImage(MemoryImage & image) const {
      // If there is no memory in the image, stop.
      if (!image.memory) return;

      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(image); }
    }

    /// Destruct and delete all memomry assocated with this MemoryImage.
    void ClearImage(MemoryImage & image) const {
      // If this MemoryImage is already clear, stop.
      if (!image.memory) return;

      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(image); }

      // Clean up image object.
      image.memory.DeleteArray();
      image.memory = nullptr;
      image.mem_size = 0;
    }

    void CopyImage(const MemoryImage & from_image, MemoryImage & to_image) const {
      emp_assert(&from_image.data_map == &to_image.data_map);

      DestructImage(to_image);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawResize(from_image.mem_size);
      to_image.RawCopy(from_image);
      for (auto & c : copy_constructors) { c(from_image, to_image); }
    }

    // Move contents from one image to another.  Size must already be setup!
    void MoveImageContents(MemoryImage & from_image, MemoryImage & to_image) const {
      emp_assert(to_image.GetSize() >= from_image.GetSize());
      emp_assert(&from_image.data_map == &to_image.data_map);

      DestructImage(to_image);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawCopy(from_image);
      for (auto & c : move_constructors) { c(from_image, to_image); }
    }

    void Initialize(MemoryImage & image) const { CopyImage(default_image, image); }

  };

}

#endif
