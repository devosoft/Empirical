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
#include "../meta/type_traits.h"
#include "../tools/FunctionSet.h"
#include "../tools/string_utils.h"
#include "../tools/NullStream.h"

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
      : filename(in_filename), os(
      #ifdef EMSCRIPTEN
      new emp::NullStream()
      #else
      new std::ofstream(in_filename)
      #endif
      ), funs(), keys(), descs()
      , timing_fun([](size_t){return true;})
      , line_begin(b), line_spacer(s), line_end(e) { ; }
    DataFile(std::ostream & in_os,
             const std::string & b="", const std::string & s=",", const std::string & e="\n")
      : filename(), os(&in_os), funs(), keys(), descs(), timing_fun([](size_t){return true;})
      , line_begin(b), line_spacer(s), line_end(e) { ; }

    DataFile(const DataFile &) = default;
    DataFile(DataFile &&) = default;
    virtual ~DataFile() { os->flush(); }

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
    virtual void PrintHeaderKeys() {
      *os << line_begin;
      for (size_t i = 0; i < keys.size(); i++) {
        if (i > 0) *os << line_spacer;
        *os << keys[i];
      }
      *os << line_end;
      os->flush();
    }

    /// Print a header containing comments describing all of the columns
    virtual void PrintHeaderComment(const std::string & cstart = "# ") {
      for (size_t i = 0; i < keys.size(); i++) {
        *os << cstart << i << ": " << descs[i] << " (" << keys[i] << ")" << std::endl;
      }
      os->flush();
    }

    /// Run all of the functions and print the results as a new line in the file
    virtual void Update() {
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
    virtual void Update(size_t update) { if (timing_fun(update)) Update(); }


    /// If a function takes an ostream, pass in the correct one.
    /// Generic function for adding a column to the DataFile. In practice, you probably
    /// want to call one of the more specific ones.
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
    size_t AddCurrent(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
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
    size_t AddMean(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetMean();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the median value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddMedian(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetMedian();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the Percentile value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddPercentile(DataNode<VAL_TYPE, MODS...> & node, const double pct=100.0, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull, pct](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetPercentile(pct);
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the total value from the DataNode @param node.
    /// Requires that @param node have the data::Range or data::FullRange modifier.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddTotal(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
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
    size_t AddMin(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
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
    size_t AddMax(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetMax();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the variance from the DataNode @param node
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddVariance(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetVariance();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the standard deviation from the DataNode @param node
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddStandardDeviation(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetStandardDeviation();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the skewness from the DataNode @param node
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddSkew(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetSkew();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add a function that always pulls the kurtosis from the DataNode @param node
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddKurtosis(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        os << node.GetKurtosis();
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }

    /// Add multiple functions that pull common stats measurements (mean, variance, min, and max)
    /// from the DataNode @param node.
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    /// @param key and @param desc will have the name of the stat appended to the beginning.
    /// Note: excludes standard deviation, because it is easily calculated from variance
    template <typename VAL_TYPE, emp::data... MODS>
    void AddStats(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      AddMean(node, "mean_" + key, "mean of " + desc, reset, pull);
      AddMin(node, "min_" + key, "min of " + desc, reset, pull);
      AddMax(node, "max_" + key, "max of " + desc, reset, pull);
      AddVariance(node, "variance_" + key, "variance of " + desc, reset, pull);
    }


    /// Add multiple functions that pull all stats measurements (mean, variance, min, max,
    /// skew, and kurtosis) from the DataNode @param node
    /// Requires that @param node have the data::Stats or data::FullStats modifier.
    /// @param key and @param desc will have the name of the stat appended to the beginning.
    /// Note: excludes standard deviation, because it is easily calculated from variance
    template <typename VAL_TYPE, emp::data... MODS>
    void AddAllStats(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      AddStats(node, key, desc, reset, pull);
      AddSkew(node, "skew_" + key, "skew of " + desc, reset, pull);
      AddKurtosis(node, "kurtosis_" + key, "kurtosis of " + desc, reset, pull);
    }

    /// Add a function that always pulls the count of the @param bin_id 'th bin of the histogram
    /// from @param node. Requires that @param node have the data::Histogram modifier and at least
    /// @bin_id bins.
    /// If @param reset is set true, we will call Reset on that DataNode after pulling the
    /// current value from the node
    template <typename VAL_TYPE, emp::data... MODS>
    size_t AddHistBin(DataNode<VAL_TYPE, MODS...> & node, size_t bin_id, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun =
        [&node,bin_id,reset, pull](std::ostream & os){
          if (pull) node.PullData();
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
    size_t AddInferiority(DataNode<VAL_TYPE, MODS...> & node, const std::string & key="", const std::string & desc="", const bool & reset=false, const bool & pull=false) {
      std::function<fun_t> in_fun = [&node, reset, pull](std::ostream & os){
        if (pull) node.PullData();
        VAL_TYPE inf = (node.GetMax() == 0) ? 0 : (node.GetMean() / node.GetMax());
        os << inf;
        if (reset) node.Reset();
      };
      return Add(in_fun, key, desc);
    }
  };


  template <typename container_t>
  class ContainerDataFile;

  // This handles all possible forms of pointers to containers.
  namespace internal {

    template <typename container_t>
    struct update_impl {
      void Update(ContainerDataFile<container_t> * df) {
        using data_t = typename container_t::value_type;
        for (const data_t & d : df->GetCurrentRows()) {
          df->OutputLine(d);
        }
      }
    };

    template <typename container_t>
    struct update_impl<Ptr<container_t>> {
      void Update(ContainerDataFile<Ptr<container_t>> * df) {
        using data_t = typename remove_ptr_type<container_t>::type::value_type;

        for (const data_t & d : *(df->GetCurrentRows())) {
          df->OutputLine(d);
        }
      }
    };

    template <typename container_t>
    struct update_impl<container_t*> {
      void Update(ContainerDataFile<container_t*> * df) {
        using data_t = typename remove_ptr_type<container_t>::type::value_type;

        for (const data_t & d : *(df->GetCurrentRows())) {
          df->OutputLine(d);
        }
      }
    };

  }

  /// Sometimes you may want a data file where a set
  /// of functions is run on every item in a container
  /// every time you write to the file. ContainerDataFiles do that.
  ///
  /// Note: CONTAINER type can be a pointer to a container and the
  /// datafile will handle derefeferencing it appropriately.

  template <typename CONTAINER>
  class ContainerDataFile : public DataFile {
    protected:
    // The container type cannot be a reference
    using container_t = typename std::remove_reference<CONTAINER>::type;
    using raw_container_t = typename remove_ptr_type<container_t>::type;
    using data_t = typename raw_container_t::value_type;
    using container_fun_t = void(std::ostream &, data_t);
    using fun_update_container_t = std::function<container_t(void)>;

    fun_update_container_t update_container_fun;

    container_t current_rows;
    FunctionSet<container_fun_t> container_funs;
    emp::vector<std::string> container_keys;
    emp::vector<std::string> container_descs;

  public:

    ContainerDataFile(const std::string & filename,
             const std::string & b="", const std::string & s=",", const std::string & e="\n")
             : DataFile(filename, b, s, e), update_container_fun(), current_rows() {;}

    ~ContainerDataFile() {;}

    /// Tell this file what function to run to update the contents of the
    /// container that data is being calculated on.
    void SetUpdateContainerFun(const fun_update_container_t fun) {
      update_container_fun = fun;
    }

    /// Print a header containing the name of each column
    void PrintHeaderKeys() override {
      *os << line_begin;
      for (size_t i = 0; i < keys.size(); i++) {
        if (i > 0) *os << line_spacer;
        *os << keys[i];
      }
      for (size_t i = 0; i < container_keys.size(); i++) {
        if (i > 0 || keys.size() > 0) *os << line_spacer;
        *os << container_keys[i];
      }
      *os << line_end;
      os->flush();
    }

    /// Print a header containing comments describing all of the columns
    void PrintHeaderComment(const std::string & cstart = "# ") override {
      for (size_t i = 0; i < keys.size(); i++) {
        *os << cstart << i << ": " << descs[i] << " (" << keys[i] << ")" << std::endl;
      }
      for (size_t i = 0; i < container_keys.size(); i++) {
        *os << cstart << i+keys.size() << ": " << container_descs[i] << " (" << container_keys[i] << ")" << std::endl;
      }

      os->flush();
    }

    const container_t GetCurrentRows() const { return current_rows; }

    void OutputLine(const data_t d) {
      *os << line_begin;
        for (size_t i = 0; i < funs.size(); i++) {
          if (i > 0) *os << line_spacer;
          funs[i](*os);
        }

        for (size_t i = 0; i < container_funs.size(); i++) {
          if (i > 0 || keys.size() > 0) *os << line_spacer;
          container_funs[i](*os, d);
        }
      *os << line_end;
    }

    /// Run all of the functions and print the results as a new line in the file
    void Update() override {
      emp_assert(update_container_fun);
      current_rows = update_container_fun();
      internal::update_impl<container_t>().Update(this);
      os->flush();
    }

    /// Update the file with an additional set of lines.
    void Update(size_t update) override {
      if (timing_fun(update)) Update();
    }

    /// If a function takes an ostream, pass in the correct one.
    /// Generic function for adding a column to the DataFile. In practice, you probably
    /// want to call one of the more specific ones.
    size_t Add(const std::function<void(std::ostream &, data_t)> & fun, const std::string & key, const std::string & desc) {
      size_t id = container_funs.GetSize();
      container_funs.Add(fun);
      container_keys.emplace_back(key);
      container_descs.emplace_back(desc);
      return id;
    }

    /// Add a function that returns a value to be printed to the file.
    template <typename T>
    size_t AddContainerFun(const std::function<T(const data_t)> & fun, const std::string & key="", const std::string & desc="") {
      std::function<container_fun_t> in_fun = [fun](std::ostream & os, const data_t data){ os << fun(data); };
      return Add(in_fun, key, desc);
    }


  };

  /// Convenience function for building a container data file.
  /// @param fun is the function to call to update the container
  template <typename CONTAINER>
  ContainerDataFile<CONTAINER> MakeContainerDataFile(std::function<CONTAINER(void)> fun,
                                                    const std::string & filename,
                                                    const std::string & b="",
                                                    const std::string & s=",",
                                                    const std::string & e="\n") {
    ContainerDataFile<CONTAINER> dfile(filename, b, s, e);
    dfile.SetUpdateContainerFun(fun);
    return dfile;
  }

}

#endif
