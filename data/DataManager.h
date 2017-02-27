//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataManager handles a set of DataNode objects with the same tracking settings.

#ifndef EMP_DATA_MANAGER_H
#define EMP_DATA_MANAGER_H

#include <map>
#include <string>

#include "../base/assert.h"
#include "../tools/map_utils.h"

#include "DataNode.h"

namespace emp {

  template <typename VAL_TYPE, emp::data... MODS>
  class DataManager {
  private:
    using data_t = VAL_TYPE;
    using node_t = DataNode<data_t, MODS...>;

    std::map<std::string, node_t *> node_map;

  public:
    DataManager() = default;
    ~DataManager() {
      for (auto & x : node_map) delete x.second;
    }

    size_t GetSize() const { return node_map.size(); }
    auto & GetNodes() const { return node_map; }

    node_t & New(const std::string & name) {
      emp_assert(!Has(node_map, name), name);
      node_map[name] = new node_t;
      return *(node_map[name]);
    }

    void Delete(const std::string & name) {
      emp_assert(Has(node_map, name), name);
      node_map.erase(name);
    }

    const node_t & Get(const std::string & name) const {
      emp_assert(Has(node_map, name), name);
      return *(node_map[name]);
    }

    node_t & Get(const std::string & name) {
      emp_assert(Has(node_map, name), name);
      return *(node_map[name]);
    }

    // == Operations that forward to DataNode objects ==
    template <typename... Ts>
    void AddData(const std::string & name, Ts... extra) {
      emp_assert(Has(node_map, name), name);
      node_map[name]->Add(extra...);
    }

    void ResetAll() {
      for (auto & x : node_map) x.second->Reset();
    }
  };

}

#endif
