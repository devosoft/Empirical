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

    virtual size_t GetResetCount() const = 0;

    virtual double GetTotal() const = 0;
    virtual double GetMean() const = 0;
    virtual double GetMin() const = 0;
    virtual double GetMax() const = 0;

    virtual void PullData() = 0;
    virtual void Reset() = 0;
    virtual void PrintDebug(std::ostream & os=std::cout) = 0;
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

    size_t GetResetCount() const { return node->GetResetCount(); }

    double GetTotal() const { return node->GetTotal(); }
    double GetMean() const { return node->GetMean(); }
    double GetMin() const { return node->GetMin(); }
    double GetMax() const { return node->GetMax(); }

    void PullData() { node->PullData(); }
    void Reset() { node->Reset(); }
    void PrintDebug(std::ostream & os=std::cout) { node->PrintDebug(os); }
  };


  template <typename VAL_TYPE, emp::data... EXTRA>
  DataInterface * MakeDataInterface() {
    return new DataInterface_Impl<VAL_TYPE, EXTRA...>();
  }

}

#endif
