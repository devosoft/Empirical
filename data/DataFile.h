//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataNode objects track a set of functions to run to build an output file.

#ifndef EMP_DATA_FILE_H
#define EMP_DATA_FILE_H

#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include "../base/vector.h"
#include "../tools/assert.h"
#include "../tools/FunctionSet.h"
#include "../tools/string_utils.h"

#include "DataNode.h"

namespace emp {

  class DataFile {
  private:
    using fun_t = void(std::ostream &);

    std::ostream * os;
    FunctionSet<fun_t> funs;

    std::string spacer;  // What should we print between entries?
    std::string eol;     // What should we print at the end of each line?

  public:
    DataFile(const std::string & filename, const std::string & s=", ", const std::string & e="\n")
      : os(new std::ofstream(filename)) { ; }
    DataFile(std::ostream & in_os, const std::string & s=", ", const std::string & e="\n")
      : os(&in_os) { ; }
    ~DataFile() { ; }

    void Update() {
      for (size_t i = 0; i < funs.size(); i++) {
        if (i > 0) *os << spacer;
        funs[i](*os);
      }
      *os << eol;
    }

    // If a function takes an ostream, pass in the correct one.
    void AddInput(const std::function<fun_t> & fun) { funs.Add(fun); }

    // If a function writes to a file directly, let it do so.
    template <typename T> void AddInput(const std::function<T()> & fun) {
      std::function<fun_t> in_fun = [fun](std::ostream & os){ os << fun(); };
      funs.Add(in_fun);
    }

    // Add various types of data from DataNodes
    template <typename VAL_TYPE, emp::data... MODS>
    void AddDataCurrent(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetCurrent(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddDataAve(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetAverage(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddDataTotal(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetTotal(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddDataMin(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMin(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddDataMax(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMax(); };
      funs.Add(in_fun);
    }
  };

}

#endif
