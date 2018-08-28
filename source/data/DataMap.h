/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  DataMap.h
 *  @brief DataMaps track arbitrary data by name (slow) or id (faster).
 *
 */

#ifndef EMP_DATA_MAP_H
#define EMP_DATA_MAP_H

#include <string>
#include <tuple>
#include <typeinfo>
#include <unordered_map>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/map_utils"
#include "../tools/string_utils.h"

namespace emp {

  template <typename... Ts>
  class DataMap {
  private:
    std::tuple<emp::vector<Ts>...> vecs;                    ///< Vector for each type of value stored.
    std::unordered_map<std::string, size_t> id_map;         ///< Lookup vector positions by name.
    std::unordered_map<std::string, std::string> type_map;  ///< Lookup value types by name.

  public:
    DataMap() { ; }
    DataMap(const DataMap &) = default;
    DataMap(DataMap &&) = default;
    ~DataMap() { ; }

    DataMap & operator=(const DataMap &) = default;
    DataMap & operator=(DataMap &&) = default;

    template <typename T>
    void AddEntry(const std::string & name, T value) {
      emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.
      auto & cur_vector = std::get<emp::vector<T>> vecs;  // Retrieve vector of the correct type.
      size_t pos = cur_vector.size();                     // Determine position of new entry.
      cur_vector.push_back(value);                        // Add the new value to the vector.
      id_map[name] = pos;                                 // Store the position in the id map.
      type_map[name] = typeid(T).name();                  // Store the type of this entry.
    }
  };

}

#endif
