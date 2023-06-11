/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019.
 *
 *  @file DataLayout.hpp
 *  @brief A mapping of names to variables stored in a MemoryImage.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATA_DATALAYOUT_HPP_INCLUDE
#define EMP_DATA_DATALAYOUT_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../meta/TypeID.hpp"

#include "MemoryImage.hpp"

namespace emp {

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
    bool is_locked = false;                               ///< Can this layout still be modified?

    /// Collect all of the constructors and destructors that we need to worry about.
    using copy_fun_t = std::function<void(const MemoryImage &, MemoryImage &)>;
    using move_fun_t = std::function<void(MemoryImage &, MemoryImage &)>;
    using destruct_fun_t = std::function<void(MemoryImage &)>;
    emp::vector<copy_fun_t> copy_constructors;
    emp::vector<move_fun_t> move_constructors;
    emp::vector<destruct_fun_t> destructors;

  public:
    DataLayout() = default;
    DataLayout(const DataLayout & _in)
      : id_map(_in.id_map), setting_map(_in.setting_map), image_size(_in.image_size), num_maps(1)
    { }
    //DataLayout(DataLayout &&) = default;
    ~DataLayout() { ; }

    // Existing layouts should never change out from under a DataMap.
    DataLayout & operator=(const DataLayout &) = delete;
    DataLayout & operator=(DataLayout &&) = delete;

    void IncMaps() { num_maps++; }
    void DecMaps() { num_maps--; }
    size_t GetNumMaps() const { return num_maps; }
    bool IsLocked() const { return is_locked; }

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

    /// Prevent this layout from being modified.
    void Lock() { is_locked = true; }

    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t Add(MemoryImage & base_memory,
                const std::string & name,
                const T & default_value,
                const std::string & desc="",
                const std::string & notes="") {
      emp_assert(!HasName(name), name);    // Make sure this doesn't already exist.
      emp_assert(is_locked == false);      // Cannot add to a locked layout.

      // std::cout << "\nL: Adding var '" << name
      //           << "' of type " << emp::GetTypeID<T>()
      //           << " to DataMap with " << id_map.size() << " elements"
      //           << " totalling " << image_size << " bytes."
      //           << std::endl;

      // Analyze the size of the new object and where it will go.
      constexpr const size_t obj_size = sizeof(T);
      const size_t pos = image_size;

      // Create a new image with enough room for the new object and move the old data over.
      MemoryImage new_memory(image_size + obj_size);
      MoveImageContents(base_memory, new_memory);

      // Now that the data is moved, cleanup the old image and put the new one in place.
      base_memory.RawMove(new_memory);

      // Setup this new object.
      image_size = base_memory.GetSize();
      base_memory.Construct<T>(pos, default_value);
      base_memory.init_to = image_size;

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

    // -- Manipulations of MemoryImages --

    /// Run destructors on all objects in a memory image (but otherwise leave it intact.)
    void DestructImage(MemoryImage & image) const {
      // If there is no memory in the image, stop.
      if (image.GetSize() == 0) return;

      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(image); }
      image.init_to = 0;
    }

    /// Destruct and delete all memomry assocated with this DataMap.
    void ClearImage(MemoryImage & image) const {
      // If this memory image is already clear, stop.
      if (image.GetSize() == 0) return;

      // Run destructor on contents of image and then empty it!
      emp_assert(image.GetInitSize() == image_size);
      for (auto & d : destructors) { d(image); }
      image.init_to = 0;

      // Clean up image memory.
      image.RawResize(0);
    }

    void CopyImage(const MemoryImage & from_image, MemoryImage & to_image) const {
      DestructImage(to_image);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawResize(from_image.GetSize());
      to_image.RawCopy(from_image);
      for (auto & c : copy_constructors) { c(from_image, to_image); }
      to_image.init_to = from_image.init_to;
    }

    // Move contents from one image to another.  Size must already be setup, and to_image must
    // be uninitialized (or destructed)
    void MoveImageContents(MemoryImage & from_image, MemoryImage & to_image) const {
      emp_assert(from_image.GetInitSize() >= image_size);
      emp_assert(to_image.GetSize() >= image_size);

      // Transfer over the from image and then run the required copy constructors.
      to_image.RawCopy(from_image);
      for (auto & c : move_constructors) { c(from_image, to_image); }
      to_image.init_to = image_size;  // Everything in the to image is now initialized.
      from_image.init_to = 0;         // Everything in the from image has been destructed.
    }

  };

}

#endif // #ifndef EMP_DATA_DATALAYOUT_HPP_INCLUDE
