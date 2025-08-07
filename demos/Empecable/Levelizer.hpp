#pragma once

#include <filesystem>
#include <set>

#include "../../include/emp/base/Ptr.hpp"
#include "../../include/emp/tools/String.hpp"

#include "helpers.hpp"

// Level information for a particular file.
struct LevelInfo {
  fs::path path; // Local file path.
  std::set<emp::Ptr<LevelInfo>> local_includes;
  std::set<emp::Ptr<LevelInfo>> included_from;

  static constexpr int LEVEL_UNKNOWN = -1;          // Level not-yet-calculated.
  static constexpr int LEVEL_IN_PROGRESS = -2;      // Level being calculated now (likely loop!)
  static constexpr int LEVEL_ERROR = -3;      // Level being calculated now (likely loop!)
  int include_level = LEVEL_UNKNOWN;                // Levelization tracker

  int GetLevel() {
    if (include_level == LEVEL_UNKNOWN) {
      include_level = LEVEL_IN_PROGRESS;
      return include_level = CalcLevel();
    }
    return include_level;    
  }

  int CalcLevel() const {
    int result = 0;
    for (emp::Ptr info_ptr : local_includes) {
      int inc_level = info_ptr->GetLevel();

      // If the include level is IN_PROGRESS, this must have been a circular include.
      // If the include level is ERROR, there was already a problem that is propagating.
      if (inc_level < 0) { return LEVEL_ERROR; }
      result = std::max(result, inc_level+1);
    }
    return result;
  }
};
