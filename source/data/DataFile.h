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

  /// Keep track of everything associated with periodically printing data to a file.
  /// Maintain a set of functions for calculating the desired measurements at each point in
  /// time that they are required. It also handles the formating of the file.

  class DataFile {
  protected:
    using fun_t = void(std::ostream &);
    using time_fun_t = std::function<bool(size_t)>;

    std::string filename;            ///< Name of the file that we are printing to (if one exists)
    std::ostream * os;               ///< Stream to print to.
    FunctionSet<fun_t> funs;         ///< Set of functions to call, one per column in the file.
    emp::vector<std::string> keys;   ///< Keywords associated with each column.
    emp::vector<std::string> descs;  ///< Full description for each column.
    time_fun_t timing_fun;           ///< Function to determine updates to print on (default: all)

    std::string line_begin;          ///< What should we print at the start of each line?
    std::string line_spacer;         ///< What should we print between entries?
    std::string line_end;            ///< What should we print at the end of each line?

  public:
    DataFile(const std::string & in_filename,
             const std::string & b="", const std::string & s=",", const std::string & e="\n")
      : filename(in_filename), os(new std::ofstream(in_filename)), funs(), keys(), descs()
      , timing_fun([](size_t){return true;})
      , line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(std::ostream & in_os,
             const std::string & b="", const std::string & s=",", const std::string & e="\n")
      : filename(), os(&in_os), funs(), keys(), descs(), timing_fun([](size_t){return true;})
      , line_begin(b), line_spacer(s), line_end(e) { ; }

    DataFile(const DataFile &) = default;
    DataFile(DataFile &&) = default;
    ~DataFile() { os->flush(); }

    DataFile & operator=(const DataFile &) = default;
    DataFile & operator=(DataFile &&) = default;

    /// Get the filename used for this file.
    const std::string & GetFilename() const { return filename; }

    /// Returns the string that is printed at the beginning of each line.
    const std::string & GetLineBegin() const { return line_begin; }
    /// Returns the string that is printed between elements on each line (i.e. the delimeter).
    const std::string & GetSpacer() const { return line_spacer; }
    /// Returns the string that is printed at the end of each line.
    const std::string & GetLineEnd() const { return line_end; }

    /// Provide a timing function with a bool(size_t update) signature.  The timing function is
    /// called with the update, and returns a bool to indicate if files should print this update.
    void SetTiming(time_fun_t fun) { timing_fun = fun; }

    /// Setup this file to print only once, at the specified update.  Note that this timing
    /// function can be replaced at any time, even after being triggered.
    void SetTimingOnce(size_t print_time) {
      timing_fun = [print_time](size_t update) { return update == print_time; };
    }

    /// Setup this file to print every 'step' updates.
    void SetTimingRepeat(size_t step) {
      emp_assert(step > 0);
      timing_fun = [step](size_t update) { return update % step == 0; };
    }

    /// Setup this file to print only in a specified time range, and a given frequency (step).
    void SetTimingRange(size_t first, size_t step, size_t last) {
      emp_assert(step > 0);
      emp_assert(first < last);
      timing_fun = [first,step,last](size_t update) {
	      if (update < first || update > last) return false;
	      return ((update - first) % step) == 0;
      };
    }

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

    /// If Update is called with an update number, call the full version of update only if the update value
    /// passes the timing function (that is, the timing function returns true).
    void Update(size_t update) { if (timing_fun(update)) Update(); }


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
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddCurrent(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        os << node.GetCurrent();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }


    /// Add a function that always pulls the mean value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMean(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        os << node.GetMean();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }


    /// Add a function that always pulls the total value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddTotal(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        os << node.GetTotal();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the minimum value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMin(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        os << node.GetMin();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the maximum value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMax(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        os << node.GetMax();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the count of the @param bin_id 'th bin of the histogram
    /// from @param node. Requires that @param node have the data::Histogram modifier and at least
    /// @bin_id bins.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddHistBin(DataNode<VAL_TYPE, MODS...> & node, size_t bin_id, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun =
        [&node,bin_id,reset](std::ostream & os){
          os << node.GetHistCount(bin_id);
          if (reset) node.Reset();
        };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the inferiority (mean divided by max) from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddInferiority(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false) {
      std::function<fun_t> in_fun = [&node, reset](std::ostream & os){
        VAL_TYPE inf = (node.GetMax() == 0) ? 0 : (node.GetMean() / node.GetMax());
        os << inf;
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }
  };

}

#endif
