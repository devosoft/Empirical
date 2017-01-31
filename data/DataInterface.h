//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataInterface is a *generic* interface to a DataNode


#ifndef EMP_DATA_INTERFACE_H
#define EMP_DATA_INTERFACE_H

#include "DataNode.h"

namespace emp {

  class DataInterface {
  private:
  public:
    virtual ~DataInterface() { ; }
  };

  template <typename VAL_TYPE, emp::data... EXTRA>
  class DataInterface_Impl : public DataInterface {
  public:
    using node_t = DataNode<VAL_TYPE, EXTRA...>;

  private:
    node_t * node;
    bool owner;

  public:
    DataInterface_Impl() : node(new node_t), owner(true) { ; }
    DataInterface_Impl(node_t * n) : node(n), owner(false) { ; }
    ~DataInterface_Impl() { if (owner) delete node; }
  };


  template <typename VAL_TYPE, emp::data... EXTRA>
  DataInterface * MakeDataInterface() {
    return new DataInterface_Impl<VAL_TYPE, EXTRA...>();
  }

}

#endif
