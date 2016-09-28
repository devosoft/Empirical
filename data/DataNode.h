//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataNode objects track a specific type of data over the course of a run.
//
//  DataNodes are templated objects that must specify a stored type and provide multiple types of
//  internal classes.  By default, the stored type may be anything, but stats require a type that
//  math can be performed on.  The internal classed are:
//
//  DataCollect : How should new input data be found?
//  DataStore   : What information should be kept about the values passing through?
//  DataProcess : How should data be processed on Reset or stored in hte long term?
//
//  More details on the specific options for each:
//
//  DataCollect      : All data needs to be sent from where it is generated.
//  DataCollect_Pull : Maintain a function to request new data (either individual values or sets)
//
//  DataStore        : Keep the most recent input value and a count of inputs since the last Reset()
//  DataStore_Stats  : Maintain basic stats (min, max, input count, mean, standard deviation,
//                     skew, kertosis), as values are supplied, but not individual values.
//  DataStore_Full   : Stores ALL values that are provided since last Reset()
//
//  DataProcess      : Setup a signal to trigger on each reset.
//                     * Actions can clean up data.
//                     * Actions can control prior update's data stored.
//                     * Actions can send data to a stream
//                       (or stats automatically have a stream that, if non-null data is sent to?)

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
