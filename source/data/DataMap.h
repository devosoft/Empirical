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
#include <tuple>
#include <typeinfo>
#include <unordered_map>

#include "MemoryImage.h"

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../meta/TypeID.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  template <typename IMAGE_T>
  class DataMapBase {
  public:
    using this_t = DataMapBase<IMAGE_T>;
    using image_t = IMAGE_T;

    struct SettingInfo {
      emp::TypeID type;   ///< Type name as converted to string by typeid()
      std::string name;   ///< Name of this setting.
      std::string desc;   ///< Full description of this setting.
      std::string notes;  ///< Any additional notes about this setting.
    };

  protected:
    IMAGE_T default_image;                                ///< Memory image for these data.
    std::unordered_map<std::string, size_t> id_map;       ///< Lookup vector positions by name.
    std::unordered_map<size_t, SettingInfo> setting_map;  ///< Lookup setting info by id.

    /// Collect all of the constructors and destructors that we need to worry about.
    using cconstruct_fun_t = std::function<void(IMAGE_T &, IMAGE_T &)>;
    using destruct_fun_t = std::function<void(IMAGE_T &)>;
    emp::vector<cconstruct_fun_t> copy_constructors;
    emp::vector<destruct_fun_t> destructors;

  public:
    DataMapBase() { ; }
    // DataMapBase(const DataMapBase &) = default;
    DataMapBase(DataMapBase &&) = default;
    ~DataMapBase() { ClearImage(default_image); }

    // DataMapBase & operator=(const DataMapBase &) = default;
    DataMapBase & operator=(DataMapBase &&) = default;

    /// Lookup the unique idea for an entry.
    size_t GetID(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return id_map.find(name)->second;
    }

    /// Lookup the type of an entry.
    emp::TypeID GetType(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return setting_map.find(GetID(name))->second.type;
    }

    /// Add a new variable with a specified type, name and value.
    template <typename T>
    size_t Add(const std::string & name,
	     T default_value,
	     const std::string & desc="",
	     const std::string & notes="") {
      emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.

      // Setup the default version of this object and save its position.
      size_t pos = default_image.template AddObject<T>(default_value);

      // Store the position in the id map.
      id_map[name] = pos;

      // Store all of the other settings for this object.
      setting_map[pos] = { emp::GetTypeID<T>(), name, desc, notes };

      return pos;
    }

    const auto & GetDefaultImage() const { return default_image; }

    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(size_t pos) {
      emp_assert(emp::Has(setting_map, pos), setting_map.size(), pos, default_image.size());
      emp_assert(setting_map[pos].type == emp::GetTypeID<T>());
      return default_image.template GetRef<T>(pos);
    }

    template <typename T>
    T & GetDefault(const std::string & name) { return GetDefault<T>(GetID(name)); }

    /// Retrieve a variable from an image by its type and position.
    template <typename T>
    T & Get(IMAGE_T & image, size_t pos) {
      emp_assert(emp::Has(setting_map, pos), setting_map.size(), pos, default_image.size());
      emp_assert(setting_map[pos].type == emp::GetTypeID<T>());
      image.template GetRef<T>(pos);
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(size_t pos) const {
      emp_assert(emp::Has(setting_map, pos), setting_map.size(), pos, default_image.size());
      emp_assert(setting_map[pos].type == emp::GetTypeID<T>());
      default_image.template GetRef<T>(pos);
    }

    template <typename T>
    const T & GetDefault(const std::string & name) const { return GetDefault<T>(GetID(name)); }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T>
    const T & Get(IMAGE_T & image, size_t pos) const {
      emp_assert(emp::Has(setting_map, pos), setting_map.size(), pos, default_image.size());
      emp_assert(setting_map[pos].type == emp::GetTypeID<T>());
      image.template GetRef<T>(pos);
    }


    // == Retrivals by name instead of by ID (slower!) ==


    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(std::string & name) {
      emp_assert(emp::Has(id_map, name));
      default_image.template GetRef<T>(GetID(name));
    }

    /// Retrieve a variable from an image by its type and position.
    template <typename T>
    T & Get(IMAGE_T & image, std::string & name) {
      emp_assert(emp::Has(id_map, name));
      image.template GetRef<T>(GetID(name));
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      default_image.template GetRef<T>(GetID(name));
    }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T>
    const T & Get(IMAGE_T & image, std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      image.template GetRef<T>(GetID(name));
    }



    /// Manipulations of images

    void ClearImage(IMAGE_T & image) {
      // Run destructor on contents of image and then empty it!
      for (auto & d : destructors) { d(image); }
      image.resize(0);
    }

    void Initialize(IMAGE_T & image) {
      // Transfer over the default image and then run the required copy constructors.
      image.RawCopy(default_image);
      for (auto & c : copy_constructors) { c(default_image, image); }
    }

    template <typename IMAGE1_T, typename IMAGE2_T>
    void CopyImage(IMAGE1_T & from_image, IMAGE2_T to_image) {
      // Transfer over the from image and then run the required copy constructors.
      to_image.RawCopy(from_image);
      for (auto & c : copy_constructors) { c(from_image, to_image); }
    }


  };

  using DataMap = DataMapBase<emp::MemoryVector>;

  template <size_t SIZE>
  using DataMapFixed = DataMapBase<emp::MemoryArray<SIZE>>;
}

#endif
