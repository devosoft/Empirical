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
  };

  template <typename VAL_TYPE, typename... EXTRA>
  class DataInterface_Impl : public DataInterface {
  };

  
  template <typename VAL_TYPE, typename... EXTRA>
  DataInterface * MakeDataInterface() { return nullptr; }

}

#endif
