/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  DataFile.h
 *  @brief DataFile objects use DataNode objects to dynamically fill out file contents.
 */

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

  /** This class keeps track of everything associated with periodically printing data to a file.
   * It maintains a set of functions for calculating the desired measurements at each point in
   * time that they are required. It also handles the formating of the file.
   */
  class DataFile {
  protected:
    using fun_t = void(std::ostream &);

    std::ostream * os;
    FunctionSet<fun_t> funs;
    emp::vector<std::string> keys;
    emp::vector<std::string> descs;

    std::string line_begin;   ///< What should we print at the start of each line?
    std::string line_spacer;  ///< What should we print between entries?
    std::string line_end;     ///< What should we print at the end of each line?

  public:
    DataFile(const std::string & filename,
             const std::string & b="", const std::string & s=", ", const std::string & e="\n")
      : os(new std::ofstream(filename)), funs(), keys(), descs()
      , line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(std::ostream & in_os,
             const std::string & b="", const std::string & s=", ", const std::string & e="\n")
      : os(&in_os), funs(), keys(), descs(), line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(const DataFile &) = default;
    DataFile(DataFile &&) = default;
    ~DataFile() { os->flush(); }

    DataFile & operator=(const DataFile &) = default;
    DataFile & operator=(DataFile &&) = default;

    /// Returns the string that is printed at the beginning of each line.
    const std::string & GetLineBegin() const { return line_begin; }
    /// Returns the string that is printed between elements on each line (i.e. the delimeter).
    const std::string & GetSpacer() const { return line_spacer; }
    /// Returns the string that is printed at the end of each line.
    const std::string & GetLineEnd() const { return line_end; }

    /// Print @param _in at the beginning of each line.
    void SetLineBegin(const std::string & _in) { line_begin = _in; }
    /// Print @param _in between elements (i.e. make @param _in the delimeter).
    void SetSpacer(const std::string & _in) { line_spacer = _in; }
    /// Print @param _in at the end of each line.
    void SetLineEnd(const std::string & _in) { line_end = _in; }
    /// Set line begin character (@param b), column delimeter (@param s), and line end character (@param e)
    void SetupLine(const std::string & b, const std::string & s, const std::string & e) {
      line_begin = b;
      line_spacer = s;
      line_end = e;
    }

    /// Print a header containing the name of each column
    void PrintHeaderKeys() {
      *os << line_begin;
      for (size_t i = 0; i < keys.size(); i++) {
        if (i > 0) *os << line_spacer;
        *os << keys[i];
      }
      *os << line_end;
      os->flush();
    }

    /// Print a header containing comments describing all of the columns
    void PrintHeaderComment(const std::string & cstart = "# ") {
      for (size_t i = 0; i < keys.size(); i++) {
        *os << cstart << i << ": " << descs[i] << " (" << keys[i] << ")" << std::endl;
      }
      os->flush();
    }

    /// Run all of the functions and print the results as a new line in the file
    void Update() {
      *os << line_begin;
      for (size_t i = 0; i < funs.size(); i++) {
        if (i > 0) *os << line_spacer;
        funs[i](*os);
      }
      *os << line_end;
      os->flush();
    }

    /// If a function takes an ostream, pass in the correct one.
    /// Generic function for adding a column to the DataFile. In practice, you probably
    /// want to call one of the more specific ones. */
    size_t Add(const std::function<void(std::ostream &)> & fun, const std::string & key, const std::string & desc) {
      size_t id = funs.GetSize();
      funs.Add(fun);
      keys.emplace_back(key);
      descs.emplace_back(desc);
      return id;
    }

    /// Add a function that returns a value to be printed to the file.
    template <typename T>
    size_t AddFun(const std::function<T()> & fun, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [fun](std::ostream & os){ os << fun(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always prints the current value of @param var.
    template <typename T>
    size_t AddVar(const T & var, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&var](std::ostream & os){ os << var; };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the current value from the DataNode @param node.
    /// Requires that @param node have the data::Current modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddCurrent(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetCurrent(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the mean value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMean(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMean(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the total value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddTotal(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetTotal(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the minimum value from the DataNode @param node
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMin(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMin(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the maximum value from the DataNode @param node
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMax(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){ os << node.GetMax(); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the count of the @param bin_id 'th bin of the histogram
    /// from @param node. Requires that @param node have the data::Histogram modifier and at least
    /// @bin_id bins.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddHistBin(DataNode<VAL_TYPE, MODS...> & node, size_t bin_id, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun =
        [&node,bin_id](std::ostream & os){ os << node.GetHistCount(bin_id); };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the inferiority (mean divided by max) from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddInferiority(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="") {
      std::function<fun_t> in_fun = [&node](std::ostream & os){
        VAL_TYPE inf = (node.GetMax() == 0) ? 0 : (node.GetMean() / node.GetMax());
        os << inf;
      };
      return Add(in_fun, key, desc);
    }
  };

}

#endif
