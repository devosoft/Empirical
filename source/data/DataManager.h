/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  DataManager.h
 *  @brief DataManager handles a set of DataNode objects with the same tracking settings.
 */

#ifndef EMP_DATA_MANAGER_H
#define EMP_DATA_MANAGER_H

#include <map>
#include <string>

#include "../base/assert.h"
#include "../tools/map_utils.h"

#include "DataNode.h"

namespace emp {

  /// DataManagers handle sets of DataNode objects that all have the same tracking settings.
  template <typename VAL_TYPE, emp::data... MODS>
  class DataManager {
  private:
    using data_t = VAL_TYPE;
    using node_t = DataNode<data_t, MODS...>;

    std::map<std::string, node_t *> node_map;

  public:
    DataManager() : node_map() { ; }
    ~DataManager() {
      for (auto & x : node_map) delete x.second;
    }

    // so we can use range-based for loops
    auto begin() -> decltype(std::begin(node_map)) {
      return std::begin(node_map);
    }
    // so we can use range-based for loops
    auto end() -> decltype(std::end(node_map)) {
      return std::end(node_map);
    }

    // so we can use range-based for loops with const
    auto begin() const -> const decltype(std::begin(node_map)) {
      return std::begin(node_map);
    }
    // so we can use range-based for loops with const
    auto end() const -> const decltype(std::end(node_map)) {
      return std::end(node_map);
    }

    /// Returns the number of DataNodes in this DataManager
    size_t GetSize() const { return node_map.size(); }
    /// Returns the std::map mapping node names (strings) to DataNodes
    auto & GetNodes() const { return node_map; }

    bool HasNode(const std::string & name) {
      return Has(node_map, name);
    }

    /// Creates and adds a new DataNode, with the name specified in @param name.
    node_t & New(const std::string & name) {
      emp_assert(!Has(node_map, name), name);
      node_map[name] = new node_t;
      return *(node_map[name]);
    }

    /// Deletes the DataNode with the name @param name.
    /// Throws an error if there is no node with that name in this manager.
    void Delete(const std::string & name) {
      emp_assert(Has(node_map, name), name);
      node_map.erase(name);
    }

    /// Returns a const reference to the node named @param name.
    /// Throws an error if there is no node with that name in this manager
    const node_t & Get(const std::string & name) const {
      emp_assert(Has(node_map, name), name);
      return *(node_map[name]);
    }

    /// Returns a reference to the node named @param name.
    /// Throws an error if there is no node with that name in this manager
    node_t & Get(const std::string & name) {
      emp_assert(Has(node_map, name), name, emp::to_string(Keys(node_map)));
      return *(node_map[name]);
    }

    // == Operations that forward to DataNode objects ==

    /** Adds data to a node in the DataManager.
     *  @param name is the node to add the data to.
     *  All subsequent arguments are the data to add to that node,
     *  and should be of whatever type all of the nodes in this maanger expect.
     *
     * Example:
     *
     * DataManager<int, data::Current, data::Range> my_data_manager;
     * my_data_manager.Add("my_node_name");
     * my_data_manager.AddData("my_node_name", 1, 2, 3, 4, 5);  */
    template <typename... Ts>
    void AddData(const std::string & name, Ts... extra) {
      emp_assert(Has(node_map, name), name, emp::to_string(Keys(node_map)));
      node_map[name]->Add(extra...);
    }

    /** Resets all nodes in this manager. For nodes without the data::Archive
     *  attribute, this clears all of their data except current. For nodes with
     *  the data::Archive attribute, this creates a new vector to start storing
     *  data, retaining the old one in the archive. */
    void ResetAll() {
      for (auto & x : node_map) x.second->Reset();
    }
  };

}

#endif
