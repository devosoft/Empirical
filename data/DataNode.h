//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataNode object track a specific type of data over the course of a run.
//
//  Raw DataNode objects are a base class that store only the most recent input and a count
//  of how many inputs it has received.  Note that the input type does not need to be
//  mathematical.
//
//  Derived classes include:
//
//  DataNode_Full  : Stores ALL values that are provided.
//  DataNode_Stats : Collect basic stats (min, max, input count, mean, standard deviation,
//                   skew, kertosis), as values are supplied, but does not keep those values.
//
//  DataNode_Pull  : Wraps one of the other data nodes (?), but know where to pull data from
//                   when needed.

#ifndef EMP_DATA_NODE_H
#define EMP_DATA_NODE_H

namespace emp {

  template <typename VAL_TYPE>
  class DataNode {
  private:
    int val_count;
    VAL_TYPE cur_val;
  public:
    virtual ~DataNode() { ; }
  };

}

#endif
