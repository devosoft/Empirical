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

  /** A generic interface to a DataNode (so that you don't need to 
   * worry about the node's exact type)*/
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
    DataInterface_Impl(const DataInterface_Impl &) = delete;
    DataInterface_Impl(DataInterface_Impl &&) = delete;
    ~DataInterface_Impl() { if (owner) delete node; }

    DataInterface_Impl & operator=(const DataInterface_Impl &) = delete;
    DataInterface_Impl & operator=(DataInterface_Impl &&) = delete;

    /** Returns the number of times this node has been reset.*/
    size_t GetResetCount() const { return node->GetResetCount(); }

    /** Returns the sum of values added since the last reset.
     * Requires that the data::Range or data::FullRange be added
     * to the DataNode */
    double GetTotal() const { return node->GetTotal(); }
    /** Returns the mean of the values added since the last reset.
     * Requires that the data::Range or data::FullRange be added
     * to the DataNode */
    double GetMean() const { return node->GetMean(); }
    /** Returns the minimum of the values added since the last reset.
     * Requires that the data::Range or data::FullRange be added
     * to the DataNode */
    double GetMin() const { return node->GetMin(); }
    /** Returns the maximum of the values added since the last reset.
     * Requires that the data::Range or data::FullRange be added
     * to the DataNode */
    double GetMax() const { return node->GetMax(); }

    /** Runs the Pull function for this DataNode and records the
     * resulting values. Requires that the data::Pull module was
     * added to this DataNode, and that a pull function was specified.
    */
    void PullData() { node->PullData(); }
    /** Reset this node. The exact effects of this depend on the
     * modules that this node has, but in general it prepares the
     * node to recieve a new set of data.
    */
    void Reset() { node->Reset(); }
    void PrintDebug(std::ostream & os=std::cout) { node->PrintDebug(os); }
  };


  template <typename VAL_TYPE, emp::data... EXTRA>
  DataInterface * MakeDataInterface() {
    return new DataInterface_Impl<VAL_TYPE, EXTRA...>();
  }

}

#endif
