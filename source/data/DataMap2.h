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
  class DataMap {
  public:
    using this_t = DataMap<IMAGE_T>;

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

  public:
    DataMap() { ; }
    // DataMap(const DataMap &) = default;
    DataMap(DataMap &&) = default;
    ~DataMap() { ; }

    // DataMap & operator=(const DataMap &) = default;
    DataMap & operator=(DataMap &&) = default;

    /// Add a new variable with a specified type, name and value.
    template <typename T>
    void Add(const std::String & name,
	     T default_value,
	     const std::string & desc="",
	     const std::string & notes="") {
      emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.

      // Setup the default version of this object and save its position.
      size_t pos = default_image.AddObject<T>(default_value);

      // Store the position in the id map.
      id_map[name] = pos;

      // Store all of the other settings for this object.
      setting_map[pos] = { emp::GetTypeID<T>(), name, desc, notes };
    }

    const auto & GetDefaultImage() const { return default_image; }

    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(size_t pos) {
      emp_assert(emp::Has(setting_map, pos) && seting_map[pos].type == emp::GetTypeID<T>());
      default_image.GetRef<T>(pos);
    }

    /// Retrieve a variable from an image by its type and position.
    template <typename T, typename IN_IMAGE_T>
    T & Get(IN_IMAGE_T & image, size_t pos) {
      emp_assert(emp::Has(setting_map, pos) && seting_map[pos].type == emp::GetTypeID<T>());
      image.GetRef<T>(pos);
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(size_t pos) const {
      emp_assert(emp::Has(setting_map, pos) && seting_map[pos].type == emp::GetTypeID<T>());
      default_image.GetRef<T>(pos);
    }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T, typename IN_IMAGE_T>
    const T & Get(IN_IMAGE_T & image, size_t pos) const {
      emp_assert(emp::Has(setting_map, pos) && seting_map[pos].type == emp::GetTypeID<T>());
      image.GetRef<T>(pos);
    }


    // == Retrivals by name instead of by ID (slower!) ==


    /// Retrieve a default variable by its type and position.
    template <typename T>
    T & GetDefault(& std::string & name) {
      emp_assert(emp::Has(id_map, name));
      default_image.GetRef<T>(id_map[name]);
    }

    /// Retrieve a variable from an image by its type and position.
    template <typename T, typename IN_IMAGE_T>
    T & Get(IN_IMAGE_T & image, & std::string & name) {
      emp_assert(emp::Has(id_map, name));
      image.GetRef<T>(id_map[name]);
    }

    // -- Constant versions of above two Get fuctions... --

    /// Retrieve a const default variable by its type and position.
    template <typename T>
    const T & GetDefault(& std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      default_image.GetRef<T>(id_map[name]);
    }

    /// Retrieve a const variable from an image by its type and position.
    template <typename T, typename IN_IMAGE_T>
    const T & Get(IN_IMAGE_T & image, & std::string & name) const {
      emp_assert(emp::Has(id_map, name));
      image.GetRef<T>(id_map[name]);
    }



    size_t GetID(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return id_map[name];
    }

    emp::TypeID GetType(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return setting_map[id_map[name]].type;
    }

    /// Manipulations of images

    template <typename IN_IMAGE_T>
    void ClearImage(IN_IMAGE_T & image) {
      // @CAO: Run destructor on contents of image.
      image.resize(0);
    }

    template <typename IN_IMAGE_T>
    void Initialize(IN_IMAGE_T & image) {
      image.RawCopy(default_image);
      // @CAO: Now do any constructors that need to be run!
    }

    template <typename IMAGE1_T, typename IMAGE2_T>
    void CopyImage(IMAGE1_T & from_image, IMAGE2_T to_image) {
      to_image.RawCopy(from_image);
      // @CAO: Now do any copy constructors that need to be run!
    }


  };

}

#endif
