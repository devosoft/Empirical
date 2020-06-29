
/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2020
 *
 *  @file  testing_utils.h
 *  @brief Utilities for testing software made with Empirical web components.
 *
 */

#ifndef WEB_TESTING_UTILS_H
#define WEB_TESTING_UTILS_H

/// Convenience function to reset empirical's D3 wrapper javascript namespace.
void ResetD3Context() {
  EM_ASM({
    emp_d3.clear_emp_d3();  // Reset the emp_d3 object tracker
  });
}

// TODO - ResetEmpContext() - Reset internal state of empirical js