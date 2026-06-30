/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/data/DataOutput.hpp
 * @brief DataOutput monitors a set of functions to output CSV data
 */

#pragma once

#ifndef INCLUDE_EMP_DATA_DATA_OUTPUT_HPP_GUARD
#define INCLUDE_EMP_DATA_DATA_OUTPUT_HPP_GUARD

#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  class DataOutput {
  private:
    emp::String            filename;
    std::filesystem::path  filepath;
    emp::Ptr<std::ostream> fp = nullptr;

    using fun_t = std::function<emp::String()>;

    emp::vector<emp::String> output_names;
    emp::vector<fun_t>       output_funs;

    void OpenFile() {
      emp_assert(!filename.empty(), "DataOutput: filename must be set before output");
      fp = emp::NewPtr<std::ofstream>(filepath / filename.str());
    }

  public:
    DataOutput() = default;
    DataOutput(const emp::String & filename) { SetFilename(filename); }

    ~DataOutput() { if (!fp.IsNull()) fp.Delete(); }

    [[nodiscard]] bool        IsActive()    const { return !filename.empty(); }
    [[nodiscard]] size_t      NumColumns()  const { return output_names.size(); }
    [[nodiscard]] emp::String GetFilename() const { return filename; }
    [[nodiscard]] std::filesystem::path GetFilepath() const { return filepath; }

    void SetFilename(const emp::String & name) {
      emp_assert(fp.IsNull(), "SetFilename called after output has started");
      filename = name;
    }

    void SetFilepath(const std::filesystem::path & path) {
      emp_assert(fp.IsNull(), "SetFilepath called after output has started");
      filepath = path;
    }

    /// Add a column using a pre-defined function that returns an emp::String.
    void AddColumn(const emp::String & name, fun_t output_fun) {
      output_names.push_back(name);
      output_funs.push_back(std::move(output_fun));
    }

    /// Add a column using a pre-defined function that returns a non-string.
    template <typename FUN_T>
    requires std::invocable<FUN_T>
    void AddColumn(const emp::String & name, FUN_T output_fun) {
      output_names.push_back(name);
      output_funs.push_back([fun=std::move(output_fun)](){ return emp::MakeString(fun()); });
    }

    /// Monitor a variable by reference.  Note: `var` must outlive this DataOutput instance.
    template <typename T>
    requires (!std::invocable<T>)
    void AddColumn(const emp::String & name, const T & var) {
      AddColumn(name, [&var]() { return emp::MakeString(var); });
    }

    void PrintHeaders() {
      if (fp.IsNull()) OpenFile();
      for (size_t i = 0; i < output_names.size(); ++i) {
        if (i) *fp << ',';
        *fp << output_names[i];
      }
      *fp << '\n';
    }

    /// Print a row of data.  Opens the file and prints headers on the first call.
    void DoOutput() {
      if (fp.IsNull()) PrintHeaders();
      OutputLine(*fp);
    }

    /// Output the next line to the terminal; print labels with each output.
    void DoTerminalOutput() {
      OutputLine(std::cout, true, "; ");
    }

    // Output a single line to the provided stream.
    // Indicate if the labels should be included on THIS line (vs. only at top)
    void OutputLine(std::ostream & os, bool include_labels=false, emp::String separator=",") {
      emp_assert(output_names.size() == output_funs.size());
      for (size_t i = 0; i < output_funs.size(); ++i) {
        if (i) os << separator;

        if (include_labels) os << output_names[i] << ":";

        // Quote non-numeric values as CSV string literals.
        emp::String output = output_funs[i]();
        if (!output.IsNumber()) output.SetLiteral();

        os << output;
      }
      os << std::endl;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATA_DATA_OUTPUT_HPP_GUARD
