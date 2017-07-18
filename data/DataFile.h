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

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/FunctionSet.h"
#include "../tools/string_utils.h"

#include "DataNode.h"

namespace emp {

  class DataFile {
  private:
    using fun_t = void(std::ostream &);

    std::ostream * os;
    FunctionSet<fun_t> funs;

    std::string line_begin; // What should we print at the start of each line?
    std::string line_spacer;     // What should we print between entries?
    std::string line_end;   // What should we print at the end of each line?

  public:
    DataFile(const std::string & filename,
             const std::string & b="", const std::string & s=", ", const std::string & e="\n")
      : os(new std::ofstream(filename)), funs(), line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(std::ostream & in_os,
             const std::string & b="", const std::string & s=", ", const std::string & e="\n")
      : os(&in_os), funs(), line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(const DataFile &) = default;
    DataFile(DataFile &&) = default;
    ~DataFile() { os->flush(); }

    DataFile & operator=(const DataFile &) = default;
    DataFile & operator=(DataFile &&) = default;

    const std::string & GetLineBegin() const { return line_begin; }
    const std::string & GetSpacer() const { return line_spacer; }
    const std::string & GetLineEnd() const { return line_end; }

    void SetLineBegin(const std::string & _in) { line_begin = _in; }
    void SetSpacer(const std::string & _in) { line_spacer = _in; }
    void SetLineEnd(const std::string & _in) { line_end = _in; }
    void SetupLine(const std::string & b, const std::string & s, const std::string & e) {
      line_begin = b;
      line_spacer = s;
      line_end = e;
    }

    void Update() {
      *os << line_begin;
      for (size_t i = 0; i < funs.size(); i++) {
        if (i > 0) *os << line_spacer;
        funs[i](*os);
      }
      *os << line_end;
    }

    // If a function takes an ostream, pass in the correct one.
    void Add(const std::function<fun_t> & fun) { funs.Add(fun); }

    // If a function writes to a file directly, let it do so.
    template <typename T> void Add(const std::function<T()> & fun) {
      std::function<fun_t> in_fun = [fun](std::ostream & os){ os << fun(); };
      funs.Add(in_fun);
    }

    // Add various types of data from DataNodes
    template <typename VAL_TYPE, emp::data... MODS>
    void AddCurrent(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetCurrent(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddMean(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMean(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddTotal(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetTotal(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddMin(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMin(); };
      funs.Add(in_fun);
    }
    template <typename VAL_TYPE, emp::data... MODS>
    void AddMax(DataNode<VAL_TYPE, MODS...> & node) {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMax(); };
      funs.Add(in_fun);
    }
  };

}

#endif
