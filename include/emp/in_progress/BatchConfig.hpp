//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A tool to control a series of runs and keep them updated.

// Development notes: currently doesn't compile because of last line

#ifndef EMP_BATCH_CONFIG_H
#define EMP_BATCH_CONFIG_H

#include <functional>

#include "../base/vector.hpp"

namespace emp {

  /// Class to managae a set of runs:
  /// RUN_T is the type of the object that contains the run info.
  /// CONFIG_T is the config object being used.

  template <typename RUN_T, typename CONFIG_T>
  class BatchConfig {
  private:
    struct RunInfo {
      size_t id;
      CONFIG_T config;

      RunInfo(size_t & _id, const CONFIG_T & _config) : id(_id), config(_config) { ; }
    };

    emp::vector<RunInfo> runs;
    size_t cur_run;

    using start_fun_t = std::function<void(const CONFIG_T &)>;
    start_fun_t start_fun;

  public:
    BatchConfig(const start_fun_t & f) : cur_run(0), start_fun(f) { ; }

    size_t GetSize() const { return runs.size(); }
    size_t GetCurRun() const { return cur_run; }
    CONFIG_T & GetConfig(size_t id) { return runs[id].config; }
    const CONFIG_T & GetConfig(size_t id) const  { return runs[id].config; }

    void AddRun(const CONFIG_T & in_config) {
      runs.emplace_back(runs.size(), in_config);
    }

    bool Start() {
      if (cur_run >= runs.size()) return false;
      start_fun(runs[cur_run].config);
      cur_run++;
      return true;
    }
  };

  // If no config type is explicitly provided, pull it from the run type.
  template <typename RUN_T> using BatchConfig = BatchConfig<RUN_T, typename RUN_T::config_t>;
}

#endif
