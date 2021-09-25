/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file DataInterface.hpp
 *  @brief DataInterface is a *generic* interface to a DataNode.
 */

#ifndef EMP_DATA_DATAINTERFACE_HPP_INCLUDE
#define EMP_DATA_DATAINTERFACE_HPP_INCLUDE


#include "DataNode.hpp"

namespace emp {

  /// A generic interface to a DataNode (so that you don't need to know the node's exact type)
  class DataInterface {
  private:
  public:
    virtual ~DataInterface() { ; }

    virtual size_t GetCount() const = 0;
    virtual size_t GetResetCount() const = 0;

    virtual double GetTotal() const = 0;
    virtual double GetMean() const = 0;
    virtual double GetMin() const = 0;
    virtual double GetMax() const = 0;

    virtual double GetVariance() const = 0;
    virtual double GetStandardDeviation() const = 0;
    virtual double GetSkew() const = 0;
    virtual double GetKurtosis() const = 0;

    virtual void PullData() = 0;
    virtual void Reset() = 0;
    virtual void PrintDebug(std::ostream & os=std::cout) = 0;

    virtual void GetName() = 0;
    virtual void GetDescription() = 0;
    virtual void GetKeyword() = 0;

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

    /// Returns the number values added to this node since the last reset.
    size_t GetCount() const { return node->GetCount(); }

    /// Returns the number of times this node has been reset.
    size_t GetResetCount() const { return node->GetResetCount(); }

    /// Returns the sum of values added since the last reset.
    /// Requires that the data::Range or data::FullRange be added to the DataNode
    double GetTotal() const { return node->GetTotal(); }

    /// Returns the mean of the values added since the last reset.
    /// Requires that the data::Range or data::FullRange be added to the DataNode
    double GetMean() const { return node->GetMean(); }

    /// Returns the minimum of the values added since the last reset.
    /// Requires that the data::Range or data::FullRange be added to the DataNode
    double GetMin() const { return node->GetMin(); }

    /// Returns the maximum of the values added since the last reset.
    /// Requires that the data::Range or data::FullRange be added to the DataNode
    double GetMax() const { return node->GetMax(); }

    /// Returns the variance of the values added since the last reset.
    /// Requires that the data::Stats or data::FullStats be added to the DataNode
    double GetVariance() const { return node->GetVariance(); }

    /// Returns the standard deviation of the values added since the last reset.
    /// Requires that the data::Stats or data::FullStats be added to the DataNode
    double GetStandardDeviation() const { return node->GetStandardDeviation(); }

    /// Returns the skewness of the values added since the last reset.
    /// Requires that the data::Stats or data::FullStats be added to the DataNode
    double GetSkew() const { return node->GetSkew(); }

    /// Returns the kurtosis of the values added since the last reset.
    /// Requires that the data::Stats or data::FullStats be added to the DataNode
    double GetKurtosis() const { return node->GetKurtosis(); }

    /// Runs the Pull function for this DataNode and records the resulting values.
    /// Requires that the data::Pull module was added to this DataNode, and that a pull function
    /// was specified.
    void PullData() { node->PullData(); }

    /// Reset this node. The exact effects of this depend on the modules that this node has,
    /// but in general it prepares the node to receive a new set of data.
    void Reset() { node->Reset(); }

    /// Print debug information about this node to @param os
    /// Useful for tracking which modifiers are included.
    void PrintDebug(std::ostream & os=std::cout) { node->PrintDebug(os); }

    /// Returns this node's name. Requires that the data::Info module was
    /// added to this DataNode, and that a name was set.
    void GetName() { node->GetName(); }

    /// Returns this node's description. Requires that the data::Info module was
    /// added to this DataNode, and that a description was set.
    void GetDescription() { node->GetDescription(); }

    /// Returns this node's keyword. Requires that the data::Info module was
    /// added to this DataNode, and that a keyword was set.
    void GetKeyword() { node->GetKeyword(); }

  };


  template <typename VAL_TYPE, emp::data... EXTRA>
  DataInterface * MakeDataInterface() {
    return new DataInterface_Impl<VAL_TYPE, EXTRA...>();
  }

}

#endif // #ifndef EMP_DATA_DATAINTERFACE_HPP_INCLUDE
